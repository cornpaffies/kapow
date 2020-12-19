/***********************************************************************
 *
 * Copyright (C) 2013-2020 Graeme Gott <graeme@gottcode.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ***********************************************************************/

#include "test_sessions.h"

#include "session.h"
#include "session_model.h"
Q_DECLARE_METATYPE(Session)

#include <QTest>

//-----------------------------------------------------------------------------

void TestSessions::addSessions_data()
{
	QTest::addColumn<QDateTime>("start");
	QTest::addColumn<QDateTime>("stop");
	QTest::addColumn<QString>("task");
	QTest::addColumn<int>("row_count");

	QTest::newRow("Simple session")
			<< QDateTime(QDate(2013, 2, 18), QTime(12, 30, 0))
			<< QDateTime(QDate(2013, 2, 18), QTime(13, 45, 0))
			<< QString("Test session.")
			<< 1;
	QTest::newRow("Session across midnight")
			<< QDateTime(QDate(2013, 2, 18), QTime(22, 30, 0))
			<< QDateTime(QDate(2013, 2, 19), QTime(2, 45, 0))
			<< QString("Test session.")
			<< 2;
	QTest::newRow("Two day session")
			<< QDateTime(QDate(2013, 2, 18), QTime(22, 30, 0))
			<< QDateTime(QDate(2013, 2, 20), QTime(2, 45, 0))
			<< QString("Test session.")
			<< 3;
}

void TestSessions::addSessions()
{
	SessionModel model;

	QFETCH(QDateTime, start);
	QFETCH(QDateTime, stop);
	QFETCH(QString, task);
	model.add(start, stop, task);

	QFETCH(int, row_count);
	QCOMPARE(model.rowCount() - 1, row_count);

	Session session = model.session(0);
	QCOMPARE(session.date(), start.date());
	QCOMPARE(session.start(), start.time());
	if (row_count > 1) {
		QCOMPARE(session.stop(), QTime(23, 59, 59));
		session = model.session(row_count - 1);
		QCOMPARE(session.date(), stop.date());
		QCOMPARE(session.start(), QTime(0, 0, 0));
	}
	QCOMPARE(session.stop(), stop.time());
	QCOMPARE(session.task(), task);
	QCOMPARE(session.isBilled(), false);
}

//-----------------------------------------------------------------------------

void TestSessions::addConflictingSessions_data()
{
	QTest::addColumn<QDateTime>("start");
	QTest::addColumn<QDateTime>("stop");
	QTest::addColumn<QString>("task");
	QTest::addColumn<QDateTime>("conflict_start");
	QTest::addColumn<QDateTime>("conflict_stop");
	QTest::addColumn<QString>("conflict_task");

	QTest::newRow("Prevent duplicate sessions")
			<< QDateTime(QDate(2013, 2, 18), QTime(12, 30, 0))
			<< QDateTime(QDate(2013, 2, 18), QTime(13, 45, 0))
			<< QString("Test session.")
			<< QDateTime(QDate(2013, 2, 18), QTime(12, 30, 0))
			<< QDateTime(QDate(2013, 2, 18), QTime(13, 45, 0))
			<< QString("Conflict session.");
	QTest::newRow("Prevent overlap session start")
			<< QDateTime(QDate(2013, 2, 18), QTime(12, 30, 0))
			<< QDateTime(QDate(2013, 2, 18), QTime(13, 45, 0))
			<< QString("Test session.")
			<< QDateTime(QDate(2013, 2, 18), QTime(11, 0, 0))
			<< QDateTime(QDate(2013, 2, 18), QTime(13, 0, 0))
			<< QString("Conflict session.");
	QTest::newRow("Prevent overlap session end")
			<< QDateTime(QDate(2013, 2, 18), QTime(12, 30, 0))
			<< QDateTime(QDate(2013, 2, 18), QTime(13, 45, 0))
			<< QString("Test session.")
			<< QDateTime(QDate(2013, 2, 18), QTime(13, 0, 0))
			<< QDateTime(QDate(2013, 2, 18), QTime(14, 0, 0))
			<< QString("Conflict session.");
	QTest::newRow("Prevent overlap entire session")
			<< QDateTime(QDate(2013, 2, 18), QTime(12, 30, 0))
			<< QDateTime(QDate(2013, 2, 18), QTime(13, 45, 0))
			<< QString("Test session.")
			<< QDateTime(QDate(2013, 2, 18), QTime(11, 0, 0))
			<< QDateTime(QDate(2013, 2, 18), QTime(14, 0, 0))
			<< QString("Conflict session.");
	QTest::newRow("Prevent overlap inside session")
			<< QDateTime(QDate(2013, 2, 18), QTime(12, 30, 0))
			<< QDateTime(QDate(2013, 2, 18), QTime(13, 45, 0))
			<< QString("Test session.")
			<< QDateTime(QDate(2013, 2, 18), QTime(13, 0, 0))
			<< QDateTime(QDate(2013, 2, 18), QTime(13, 30, 0))
			<< QString("Conflict session.");
}

void TestSessions::addConflictingSessions()
{
	SessionModel model;

	QFETCH(QDateTime, start);
	QFETCH(QDateTime, stop);
	QFETCH(QString, task);
	model.add(start, stop, task);

	QFETCH(QDateTime, conflict_start);
	QFETCH(QDateTime, conflict_stop);
	QFETCH(QString, conflict_task);
	model.add(conflict_start, conflict_stop, conflict_task);

	QCOMPARE(model.rowCount() - 1, 1);
}

//-----------------------------------------------------------------------------

void TestSessions::addMultipleSessions_data()
{
	QTest::addColumn<QVariantList>("sessions");
	QTest::addColumn<QVariantList>("result");

	QTest::newRow("Sorted")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		);
	QTest::newRow("Sorted reverse")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
		)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		);
	QTest::newRow("Unsorted")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		);
	QTest::newRow("Sessions with some billed")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		);
	QTest::newRow("Prevent adding to billed")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(10, 0, 0), QTime(11, 0, 0), "Test session.", false))
		)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		);
}

void TestSessions::addMultipleSessions()
{
	SessionModel model;

	QFETCH(QVariantList, sessions);
	for (int i = 0; i < sessions.count(); ++i) {
		model.add(sessions.at(i).value<Session>());
	}

	QFETCH(QVariantList, result);
	QCOMPARE(model.rowCount() - 1, result.count());
	for (int i = 0; i < result.count(); ++i) {
		QCOMPARE(model.session(i), result.at(i).value<Session>());
	}
}

//-----------------------------------------------------------------------------

void TestSessions::addSessionsFixConflict_data()
{
	QTest::addColumn<QList<Session>>("sessions");
	QTest::addColumn<QDateTime>("start");
	QTest::addColumn<QDateTime>("stop");
	QTest::addColumn<QString>("task");
	QTest::addColumn<QList<Session>>("result");

	QTest::newRow("No sessions")
		<< QList<Session>()
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 18), QTime(15, 45, 0))
		<< QString("New session.")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(14, 30, 0), QTime(15, 45, 0), "New session.", false)
		};
	QTest::newRow("Session before")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false)
		}
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 18), QTime(15, 45, 0))
		<< QString("New session.")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2020, 12, 18), QTime(14, 30, 0), QTime(15, 45, 0), "New session.", false)
		};
	QTest::newRow("Session after")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		}
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 18), QTime(15, 45, 0))
		<< QString("New session.")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(14, 30, 0), QTime(15, 45, 0), "New session.", false),
			Session(QDate(2020, 12, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		};
	QTest::newRow("Intersect end")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(15, 30, 0), QTime(17, 45, 0), "Test session.", false)
		}
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 18), QTime(15, 45, 0))
		<< QString("New session.")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(14, 30, 0), QTime(15, 29, 59), "New session.", false),
			Session(QDate(2020, 12, 18), QTime(15, 30, 0), QTime(17, 45, 0), "Test session.", false)
		};
	QTest::newRow("Intersect end next day")
		<< QList<Session>{
			Session(QDate(2020, 12, 19), QTime(15, 30, 0), QTime(17, 45, 0), "Test session.", false)
		}
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 19), QTime(15, 45, 0))
		<< QString("New session.")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(14, 30, 0), QTime(23, 59, 59), "New session.", false),
			Session(QDate(2020, 12, 19), QTime(0, 0, 0), QTime(15, 29, 59), "New session.", false),
			Session(QDate(2020, 12, 19), QTime(15, 30, 0), QTime(17, 45, 0), "Test session.", false)
		};
	QTest::newRow("Intersect start")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(12, 30, 0), QTime(15, 45, 0), "Test session.", false)
		}
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 18), QTime(15, 45, 0))
		<< QString("New session.")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(12, 30, 0), QTime(15, 45, 0), "Test session.", false)
		};
	QTest::newRow("Intersect start previous day")
		<< QList<Session>{
			Session(QDate(2020, 12, 17), QTime(12, 30, 0), QTime(15, 45, 0), "Test session.", false)
		}
		<< QDateTime(QDate(2020, 12, 17), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 18), QTime(15, 45, 0))
		<< QString("New session.")
		<< QList<Session>{
			Session(QDate(2020, 12, 17), QTime(12, 30, 0), QTime(15, 45, 0), "Test session.", false)
		};
	QTest::newRow("Overlap single")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(15, 0, 0), QTime(15, 15, 0), "Test session.", false)
		}
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 18), QTime(17, 45, 0))
		<< QString("New session.")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(14, 30, 0), QTime(14, 59, 59), "New session.", false),
			Session(QDate(2020, 12, 18), QTime(15, 0, 0), QTime(15, 15, 0), "Test session.", false)
		};
	QTest::newRow("Overlap single next day")
		<< QList<Session>{
			Session(QDate(2020, 12, 19), QTime(15, 0, 0), QTime(15, 15, 0), "Test session.", false)
		}
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 19), QTime(17, 45, 0))
		<< QString("New session.")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(14, 30, 0), QTime(23, 59, 59), "New session.", false),
			Session(QDate(2020, 12, 19), QTime(0, 0, 0), QTime(14, 59, 59), "New session.", false),
			Session(QDate(2020, 12, 19), QTime(15, 0, 0), QTime(15, 15, 0), "Test session.", false)
		};
	QTest::newRow("Overlap single previous day")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(15, 0, 0), QTime(15, 15, 0), "Test session.", false)
		}
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 19), QTime(17, 45, 0))
		<< QString("New session.")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(14, 30, 0), QTime(14, 59, 59), "New session.", false),
			Session(QDate(2020, 12, 18), QTime(15, 0, 0), QTime(15, 15, 0), "Test session.", false)
		};
	QTest::newRow("Overlap multiple")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(15, 0, 0), QTime(15, 15, 0), "First session.", false),
			Session(QDate(2020, 12, 18), QTime(16, 0, 0), QTime(16, 15, 0), "Second session.", false)
		}
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 18), QTime(17, 45, 0))
		<< QString("New session.")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(14, 30, 0), QTime(14, 59, 59), "New session.", false),
			Session(QDate(2020, 12, 18), QTime(15, 0, 0), QTime(15, 15, 0), "First session.", false),
			Session(QDate(2020, 12, 18), QTime(16, 0, 0), QTime(16, 15, 0), "Second session.", false)
		};
	QTest::newRow("Overlap multiple next day")
		<< QList<Session>{
			Session(QDate(2020, 12, 19), QTime(15, 0, 0), QTime(15, 15, 0), "First session.", false),
			Session(QDate(2020, 12, 19), QTime(16, 0, 0), QTime(16, 15, 0), "Second session.", false)
		}
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 19), QTime(17, 45, 0))
		<< QString("New session.")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(14, 30, 0), QTime(23, 59, 59), "New session.", false),
			Session(QDate(2020, 12, 19), QTime(0, 0, 0), QTime(14, 59, 59), "New session.", false),
			Session(QDate(2020, 12, 19), QTime(15, 0, 0), QTime(15, 15, 0), "First session.", false),
			Session(QDate(2020, 12, 19), QTime(16, 0, 0), QTime(16, 15, 0), "Second session.", false)
		};
	QTest::newRow("Overlap multiple previous day")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(15, 0, 0), QTime(15, 15, 0), "First session.", false),
			Session(QDate(2020, 12, 18), QTime(16, 0, 0), QTime(16, 15, 0), "Second session.", false)
		}
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 19), QTime(17, 45, 0))
		<< QString("New session.")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(14, 30, 0), QTime(14, 59, 59), "New session.", false),
			Session(QDate(2020, 12, 18), QTime(15, 0, 0), QTime(15, 15, 0), "First session.", false),
			Session(QDate(2020, 12, 18), QTime(16, 0, 0), QTime(16, 15, 0), "Second session.", false)
		};
}

void TestSessions::addSessionsFixConflict()
{
	SessionModel model;

	QFETCH(QList<Session>, sessions);
	for (const Session& session : sessions) {
		model.add(session);
	}

	QFETCH(QDateTime, start);
	QFETCH(QDateTime, stop);
	QFETCH(QString, task);
	model.fixConflict(start, stop);
	model.add(start, stop, task);

	QFETCH(QList<Session>, result);
	QCOMPARE(model.rowCount() - 1, result.count());
	for (int i = 0; i < result.count(); ++i) {
		QCOMPARE(model.session(i), result.at(i));
	}
}

//-----------------------------------------------------------------------------

void TestSessions::addSessionsHasConflict_data()
{
	QTest::addColumn<QList<Session>>("sessions");
	QTest::addColumn<QDateTime>("test");
	QTest::addColumn<bool>("result");

	QTest::newRow("No sessions")
		<< QList<Session>()
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< false;
	QTest::newRow("Session before")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false)
		}
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< false;
	QTest::newRow("Session after")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		}
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< false;
	QTest::newRow("Between sessions")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false),
			Session(QDate(2020, 12, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false)
		}
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< false;
	QTest::newRow("Inside session")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false)
		}
		<< QDateTime(QDate(2020, 12, 18), QTime(13, 0, 0))
		<< true;
}

void TestSessions::addSessionsHasConflict()
{
	SessionModel model;

	QFETCH(QList<Session>, sessions);
	for (const Session& session : sessions) {
		model.add(session);
	}

	QFETCH(QDateTime, test);
	QFETCH(bool, result);
	QCOMPARE(model.hasConflict(test), result);
}

//-----------------------------------------------------------------------------

void TestSessions::addSessionsMaximumDateTime_data()
{
	QTest::addColumn<QDateTime>("maximum");
	QTest::addColumn<QDateTime>("start");
	QTest::addColumn<QDateTime>("stop");
	QTest::addColumn<QString>("task");
	QTest::addColumn<QList<Session>>("result");

	QTest::newRow("Add before max")
		<< QDateTime(QDate(2020, 12, 18), QTime(16, 0, 0))
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 18), QTime(15, 45, 0))
		<< QString("New session.")
		<< QList<Session>{
			Session(QDate(2020, 12, 18), QTime(14, 30, 0), QTime(15, 45, 0), "New session.", false)
		};
	QTest::newRow("Prevent add across max")
		<< QDateTime(QDate(2020, 12, 18), QTime(15, 0, 0))
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 18), QTime(15, 45, 0))
		<< QString("New session.")
		<< QList<Session>();
	QTest::newRow("Prevent add after max")
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 0, 0))
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< QDateTime(QDate(2020, 12, 18), QTime(15, 45, 0))
		<< QString("New session.")
		<< QList<Session>();
}

void TestSessions::addSessionsMaximumDateTime()
{
	SessionModel model;

	QFETCH(QDateTime, maximum);
	model.setMaximumDateTime(maximum);

	QFETCH(QDateTime, start);
	QFETCH(QDateTime, stop);
	QFETCH(QString, task);
	model.add(start, stop, task);

	QFETCH(QList<Session>, result);
	QCOMPARE(model.rowCount() - 1, result.count());
	for (int i = 0; i < result.count(); ++i) {
		QCOMPARE(model.session(i), result.at(i));
	}
}

//-----------------------------------------------------------------------------

void TestSessions::billSessions_data()
{
	QTest::addColumn<QVariantList>("sessions");
	QTest::addColumn<QVariantList>("toggle");
	QTest::addColumn<QVariantList>("result");

	QTest::newRow("Check unbilled status")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< QVariantList()
		<< (QVariantList()
			<< false
			<< false
			<< false
			<< false
		);
	QTest::newRow("Check billed status")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", true))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", true))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< QVariantList()
		<< (QVariantList()
			<< true
			<< true
			<< true
			<< false
		);
	QTest::newRow("Bill session")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< (QVariantList()
			<< 1
		)
		<< (QVariantList()
			<< true
			<< true
			<< false
			<< false
		);
	QTest::newRow("Bill second session")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", true))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< (QVariantList()
			<< 2
		)
		<< (QVariantList()
			<< true
			<< true
			<< true
			<< false
		);
	QTest::newRow("Unbill session")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", true))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", true))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< (QVariantList()
			<< 2
		)
		<< (QVariantList()
			<< true
			<< false
			<< false
			<< false
		);
	QTest::newRow("Unbill all sessions")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", true))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", true))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< (QVariantList()
			<< 0
			<< 2
		)
		<< (QVariantList()
			<< false
			<< false
			<< false
			<< false
		);
	QTest::newRow("Toggle billed session")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", true))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< (QVariantList()
			<< 2
			<< 2
		)
		<< (QVariantList()
			<< true
			<< true
			<< true
			<< false
		);
	QTest::newRow("Toggle unbilled session")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< (QVariantList()
			<< 2
			<< 2
		)
		<< (QVariantList()
			<< false
			<< false
			<< false
			<< false
		);
}

void TestSessions::billSessions()
{
	SessionModel model;

	QFETCH(QVariantList, sessions);
	for (int i = 0; i < sessions.count(); ++i) {
		model.add(sessions.at(i).value<Session>());
	}

	QFETCH(QVariantList, toggle);
	for (int i = 0; i < toggle.count(); ++i) {
		int pos = toggle.at(i).toInt();
		model.setBilled(pos, !model.session(pos).isBilled());
	}

	QFETCH(QVariantList, result);
	QCOMPARE(model.rowCount() - 1, result.count());
	for (int i = 0; i < result.count(); ++i) {
		QCOMPARE(model.isBilled(i), result.at(i).value<bool>());
	}
}

//-----------------------------------------------------------------------------

void TestSessions::editSessions_data()
{
	QTest::addColumn<QVariantList>("sessions");
	QTest::addColumn<int>("position");
	QTest::addColumn<Session>("replacement");
	QTest::addColumn<QVariantList>("result");

	QTest::newRow("Simple edit")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		)
		<< 1
		<< Session(QDate(2013, 2, 18), QTime(15, 0, 0), QTime(15, 45, 0), "Test session.", false)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(15, 0, 0), QTime(15, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		);
	QTest::newRow("Prevent editing into conflict")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		)
		<< 1
		<< Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		);
	QTest::newRow("Prevent editing in billed")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		)
		<< 0
		<< Session(QDate(2013, 2, 18), QTime(15, 0, 0), QTime(15, 45, 0), "Test session.", false)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		);
	QTest::newRow("Prevent editing into billed")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		)
		<< 2
		<< Session(QDate(2013, 2, 18), QTime(10, 0, 0), QTime(11, 0, 0), "Test session.", false)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		);
	QTest::newRow("Prevent editing billed session")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		)
		<< 1
		<< Session(QDate(2013, 2, 18), QTime(15, 0, 0), QTime(15, 45, 0), "Test session.", false)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		);
}

void TestSessions::editSessions()
{
	SessionModel model;

	QFETCH(QVariantList, sessions);
	for (int i = 0; i < sessions.count(); ++i) {
		model.add(sessions.at(i).value<Session>());
	}

	QFETCH(int, position);
	QFETCH(Session, replacement);
	model.edit(position, replacement);

	QFETCH(QVariantList, result);
	QCOMPARE(model.rowCount() - 1, result.count());
	for (int i = 0; i < result.count(); ++i) {
		QCOMPARE(model.session(i), result.at(i).value<Session>());
	}
}

//-----------------------------------------------------------------------------

void TestSessions::editSessionsMaximumDateTime_data()
{
	QTest::addColumn<Session>("session");
	QTest::addColumn<QDateTime>("maximum");
	QTest::addColumn<Session>("replacement");
	QTest::addColumn<Session>("result");

	QTest::newRow("Before to before max")
		<< Session(QDate(2020, 12, 18), QTime(12, 0, 0), QTime(13, 45, 0), "Test session.", false)
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< Session(QDate(2020, 12, 18), QTime(11, 0, 0), QTime(12, 45, 0), "New session.", false)
		<< Session(QDate(2020, 12, 18), QTime(11, 0, 0), QTime(12, 45, 0), "New session.", false);
	QTest::newRow("After to before max")
		<< Session(QDate(2020, 12, 18), QTime(15, 0, 0), QTime(16, 45, 0), "Test session.", false)
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< Session(QDate(2020, 12, 18), QTime(11, 0, 0), QTime(12, 45, 0), "New session.", false)
		<< Session(QDate(2020, 12, 18), QTime(11, 0, 0), QTime(12, 45, 0), "New session.", false);
	QTest::newRow("Across to before max")
		<< Session(QDate(2020, 12, 18), QTime(12, 0, 0), QTime(17, 45, 0), "Test session.", false)
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< Session(QDate(2020, 12, 18), QTime(11, 0, 0), QTime(12, 45, 0), "New session.", false)
		<< Session(QDate(2020, 12, 18), QTime(11, 0, 0), QTime(12, 45, 0), "New session.", false);
	QTest::newRow("Prevent before to after max")
		<< Session(QDate(2020, 12, 18), QTime(12, 0, 0), QTime(13, 45, 0), "Test session.", false)
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< Session(QDate(2020, 12, 18), QTime(15, 0, 0), QTime(16, 45, 0), "New session.", false)
		<< Session(QDate(2020, 12, 18), QTime(12, 0, 0), QTime(13, 45, 0), "Test session.", false);
	QTest::newRow("Prevent before to across max")
		<< Session(QDate(2020, 12, 18), QTime(12, 0, 0), QTime(13, 45, 0), "Test session.", false)
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< Session(QDate(2020, 12, 18), QTime(12, 0, 0), QTime(16, 45, 0), "New session.", false)
		<< Session(QDate(2020, 12, 18), QTime(12, 0, 0), QTime(13, 45, 0), "Test session.", false);
	QTest::newRow("Prevent after to after max")
		<< Session(QDate(2020, 12, 18), QTime(15, 0, 0), QTime(16, 45, 0), "Test session.", false)
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< Session(QDate(2020, 12, 18), QTime(17, 0, 0), QTime(18, 45, 0), "New session.", false)
		<< Session(QDate(2020, 12, 18), QTime(15, 0, 0), QTime(16, 45, 0), "Test session.", false);
	QTest::newRow("Prevent after to across max")
		<< Session(QDate(2020, 12, 18), QTime(15, 0, 0), QTime(16, 45, 0), "Test session.", false)
		<< QDateTime(QDate(2020, 12, 18), QTime(14, 30, 0))
		<< Session(QDate(2020, 12, 18), QTime(12, 0, 0), QTime(16, 45, 0), "New session.", false)
		<< Session(QDate(2020, 12, 18), QTime(15, 0, 0), QTime(16, 45, 0), "Test session.", false);
}

void TestSessions::editSessionsMaximumDateTime()
{
	SessionModel model;

	QFETCH(Session, session);
	model.add(session);

	QFETCH(QDateTime, maximum);
	model.setMaximumDateTime(maximum);

	QFETCH(Session, replacement);
	model.edit(0, replacement);

	QFETCH(Session, result);
	QCOMPARE(model.session(0), result);
}

//-----------------------------------------------------------------------------

void TestSessions::removeSessions_data()
{
	QTest::addColumn<QVariantList>("sessions");
	QTest::addColumn<QVariantList>("remove");
	QTest::addColumn<QVariantList>("result");

	QTest::newRow("Remove one session")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		)
		<< (QVariantList()
			<< 1
		)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		);
	QTest::newRow("Remove two sessions in a row")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		)
		<< (QVariantList()
			<< 1
			<< 1
		)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(18, 0, 0), QTime(20, 15, 0), "Test session.", false))
		);
	QTest::newRow("Prevent removing from billed")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		)
		<< (QVariantList()
			<< 0
		)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		);
	QTest::newRow("Prevent removing billed session")
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		)
		<< (QVariantList()
			<< 1
		)
		<< (QVariantList()
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(12, 30, 0), QTime(13, 45, 0), "Test session.", false))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(14, 0, 0), QTime(14, 45, 0), "Test session.", true))
			<< QVariant::fromValue<Session>(Session(QDate(2013, 2, 18), QTime(16, 30, 0), QTime(17, 45, 0), "Test session.", false))
		);
}

void TestSessions::removeSessions()
{
	SessionModel model;

	QFETCH(QVariantList, sessions);
	for (int i = 0; i < sessions.count(); ++i) {
		model.add(sessions.at(i).value<Session>());
	}

	QFETCH(QVariantList, remove);
	for (int i = 0; i < remove.count(); ++i) {
		model.remove(remove.at(i).toInt());
	}

	QFETCH(QVariantList, result);
	QCOMPARE(model.rowCount() - 1, result.count());
	for (int i = 0; i < result.count(); ++i) {
		QCOMPARE(model.session(i), result.at(i).value<Session>());
	}
}

//-----------------------------------------------------------------------------

QTEST_MAIN(TestSessions)

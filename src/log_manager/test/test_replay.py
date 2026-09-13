import sqlite3

from log_manager.replay import LogReplay


def test_replay_loads_rows_in_order_and_keeps_intervals(tmp_path):
    database = tmp_path / 'log_00000001.db'
    connection = sqlite3.connect(database)
    connection.execute('CREATE TABLE logs (id INTEGER PRIMARY KEY, time_usec REAL, depth REAL)')
    connection.executemany(
        'INSERT INTO logs (id, time_usec, depth) VALUES (?, ?, ?)',
        [(2, 10.50, 2.0), (1, 10.00, 1.0), (3, 10.75, 3.0)],
    )
    connection.commit()
    connection.close()

    replay = LogReplay(database)

    assert [row['depth'] for row in replay.rows] == [1.0, 2.0, 3.0]
    assert replay.delay_after(0) == 0.5
    assert replay.delay_after(1) == 0.25
    assert replay.delay_after(2) is None

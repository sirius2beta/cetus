"""Read recorded sensor rows and preserve their recorded timing."""

import sqlite3
from pathlib import Path


class LogReplay:
    """An in-memory, read-only view of a mission log.

    ``time_usec`` is the historical column name.  Existing log files store a
    monotonic time in seconds in that column, so delays are returned in seconds.
    """

    def __init__(self, log_file):
        path = Path(log_file).expanduser().resolve()
        if not path.is_file():
            raise FileNotFoundError('Replay log does not exist: {}'.format(path))

        connection = sqlite3.connect('{}?mode=ro'.format(path.as_uri()), uri=True)
        connection.row_factory = sqlite3.Row
        try:
            self.rows = [dict(row) for row in
                         connection.execute('SELECT * FROM logs ORDER BY id ASC')]
        finally:
            connection.close()

        if not self.rows:
            raise ValueError('Replay log contains no rows: {}'.format(path))

    def delay_after(self, row_index):
        """Return the delay before the row following ``row_index``."""
        if row_index >= len(self.rows) - 1:
            return None
        current = self.rows[row_index].get('time_usec')
        following = self.rows[row_index + 1].get('time_usec')
        try:
            return max(0.0, float(following) - float(current))
        except (TypeError, ValueError):
            return 0.0

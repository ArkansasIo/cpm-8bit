-- osbex/db/bootstrap.sql — OSBEX 2.2 built-in database bootstrap
-- MySQL-compatible schema subset understood by the OSBEX SQL layer.
-- Load from the shell:  SQLFILE osbex/db/bootstrap.sql

-- system configuration (the OS reads these via osbex::db::Config)
CREATE TABLE config (key STRING PRIMARY KEY, value STRING);
INSERT INTO config VALUES ('os.name', 'OSBEX');
INSERT INTO config VALUES ('os.version', '2.2');
INSERT INTO config VALUES ('os.vendor', 'Osborne');
INSERT INTO config VALUES ('boot.timeout', '5');
INSERT INTO config VALUES ('shell.history', '64');
INSERT INTO config VALUES ('video.cols', '80');
INSERT INTO config VALUES ('video.rows', '25');
INSERT INTO config VALUES ('audio.enabled', 'true');
INSERT INTO config VALUES ('cpm.drives', 'A,B,C,D');
INSERT INTO config VALUES ('network.stack', 'udp');

-- example application table
CREATE TABLE users (id INT PRIMARY KEY, name STRING, admin BOOL);
INSERT INTO users VALUES (1, 'root', true);
INSERT INTO users VALUES (2, 'operator', false);

SELECT * FROM config;
SELECT * FROM users;

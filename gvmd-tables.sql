--
-- Create tables required for testing the extension, normally created by gvmd.
--

CREATE TABLE IF NOT EXISTS meta
  (id SERIAL PRIMARY KEY,
   name text UNIQUE NOT NULL,
   value text);

INSERT INTO meta(name, value) 
  VALUES ('max_hosts', 1024)
  ON CONFLICT DO NOTHING;

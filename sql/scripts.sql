DROP TABLE IF EXISTS import;
CREATE TABLE import (
  id          integer PRIMARY KEY,
  icao        character(6),
  altitude    integer,
  latitude    double precision,
  longitude   double precision ,
  heading     integer,
  velocity    integer,
  vert_rate   integer,
  timestamp   timestamp default now(),
  submitter   character varying(50),
  flight      character varying(7),
  airline     character varying(100),
  airlinecode character varying(3)
);


DROP TABLE IF EXISTS flugdaten;
DROP TABLE IF EXISTS airline;
CREATE TABLE airline (
  code character varying(3) PRIMARY KEY,
  name character varying(100)
);
CREATE TABLE flugdaten (
  id          integer PRIMARY KEY,
  icao        character(6),
  flugnr      character varying(7),
  airlinecode character varying(3) references airline(code),
  flughoehe   integer,
  breite      double precision,
  laenge      double precision ,
  richtung    integer,
  geschwindigkeit integer,
  vert_rate   integer,
  zeit        timestamp default now(),
  sender      character varying(50)
);


INSERT INTO airline (
  SELECT airlinecode, airline FROM import GROUP BY airline, airlinecode
);
INSERT INTO flugdaten (
  SELECT
  id, icao, flight, airlinecode, altitude, latitude, longitude, heading, velocity, vert_rate, timestamp, submitter
  FROM import
);


\set lat 51.316662
\set lon 6.570534
\set km 10.0


\set lat_min :lat - :km/111.1
\set lat_max :lat + :km/111.1

\set lon_min :lon - :km/111.1/cos(radians(:lat))
\set lon_max :lon + :km/111.1/cos(radians(:lat))

SELECT :lat_min, :lat_max;
SELECT :lon_min, :lon_max;

SELECT distinct(flugnr), airline.name
FROM flugdaten
LEFT JOIN airline
ON flugdaten.airlinecode = airline.code
WHERE breite >= :lat_min
AND breite <= :lat_max
AND laenge >= :lon_min
AND laenge <= :lon_max;


SELECT airline.code, avg(geschwindigkeit) AS durchschittsgeschwindigkeit
FROM flugdaten
LEFT JOIN airline
ON flugdaten.airlinecode = airline.code
GROUP BY airline.code
HAVING avg(geschwindigkeit) > (SELECT avg(geschwindigkeit) FROM flugdaten);


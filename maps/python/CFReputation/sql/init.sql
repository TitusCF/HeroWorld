CREATE TABLE IF NOT EXISTS schema(version INT);

CREATE TABLE regions(
    faction      TEXT,
    region       TEXT,    -- region name (or 'ALL') this faction controls
    influence    NUMERIC,
    CONSTRAINT   influence_range CHECK(influence BETWEEN 0 AND 1)
);

CREATE TABLE relations(
    faction      TEXT,
    race         TEXT,
    attitude     NUMERIC,
    PRIMARY KEY  (faction, race),
    CONSTRAINT   attitude_range CHECK(attitude BETWEEN -1 AND 1)
);

CREATE TABLE reputations(
    name         TEXT,          -- player name
    faction      TEXT,
    reputation   NUMERIC,
    PRIMARY KEY  (name, faction),
    CONSTRAINT   reputation_range CHECK(reputation BETWEEN -1 AND 1)
);

INSERT INTO regions VALUES
('Dragons', 'ALL', 0.4),
('Scorn', 'scorn', 0.5),
('Scorn', 'scornarena', 0.5),
('Scorn', 'scorncounty', 0.5),
('Scorn', 'scornoldcity', 0.5);

INSERT INTO relations VALUES
('Dragons', 'dragon', 1),
('Dragons', 'faerie', -1),
('Dragons', 'human', -1),
('Scorn', 'demon', -1),
('Scorn', 'dragon', -1),
('Scorn', 'giant', -1),
('Scorn', 'goblin', -1),
('Scorn', 'human', 1),
('Scorn', 'reptile', -1),
('Scorn', 'troll', -1),
('Scorn', 'undead', -1),
('Scorn', 'unnatural', -1);

INSERT INTO schema VALUES(1);

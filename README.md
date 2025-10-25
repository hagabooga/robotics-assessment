# Robotics Assessment

## Database Setup

For this assessment, I have installed PostGIS on the PostgreSQL database to handle spatial data effectively.

### Database Schema

```sql
CREATE TABLE IF NOT EXISTS inspection_group (
    id BIGINT NOT NULL,
    PRIMARY KEY (id)
);

CREATE TABLE IF NOT EXISTS inspection_region (
    id BIGINT NOT NULL,
    group_id BIGINT,
    PRIMARY KEY (id)
);

ALTER TABLE inspection_region ADD COLUMN IF NOT EXISTS coord_x FLOAT;
ALTER TABLE inspection_region ADD COLUMN IF NOT EXISTS coord_y FLOAT;
ALTER TABLE inspection_region ADD COLUMN IF NOT EXISTS category INTEGER;

ALTER TABLE inspection_region ADD COLUMN IF NOT EXISTS geom geometry(Point);
CREATE INDEX IF NOT EXISTS idx_region_geom ON inspection_region USING GIST (geom);
```

I've added a geometry column to the `inspection_region` table to store point data and created a spatial index on this column to optimize spatial queries. This setup allows for efficient querying of points within specified regions using PostGIS functions.

## Development Tools

**Package Management**: vcpkg is used for package management, ensuring that all necessary libraries for spatial data handling are included in the C++ projects.

**IDE**: Visual Studio is the IDE used for developing the C++ console applications. It is used to build and run the programs for data loading and querying.

## Projects

Both projects are developed in C++ using Visual Studio and managed with vcpkg for dependencies.

- Task1
- Task2

## Applications

**Task1.exe**: Compiled C++ console program for data loading.

**Task2.exe**: Compiled C++ console program for querying. Task2.exe includes Task 3's logical operators for querying regions based on the provided JSON input file.

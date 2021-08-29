## Width Filter

Queries that obtain big result sets(whether the queries are rogue or accidental) cause two problems:

- High memory usage on postgres clients, making them crash due to OOM.
- Exhaust bandwith on the database and middleware

This extension prevents those queries from running and transferring data.

It does so by aborting query execution at the planner level by **limiting the total width**(`average_row_size_in_bytes*estimated_number_of_rows`)
of the query.

### Example Usage

```sql
-- run the environment
-- nix-shell
-- width-filter-pg-13 psql

-- set to a low limit and reload the conf with a superuser
alter system set width_filter.max_width TO 100;
select pg_reload_conf();

-- try a query with a non-superuser
create role nosuper;
set role nosuper;

select * from generate_series(1,50);
-- ERROR:  54001: max width exceeded
-- DETAIL:  the query width of 200.000000 exceeds the max width of 100.00000

-- a less "wide" query will pass
select * from generate_series(1,10);
┌─────────────────┐
│ generate_series │
├─────────────────┤
│               1 │
│               2 │
│               3 │
│               4 │
│               5 │
│               6 │
│               7 │
│               8 │
│               9 │
│              10 │
└─────────────────┘
```

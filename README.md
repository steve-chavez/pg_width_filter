## Width Filter

Queries that obtain wide result sets can:

- Cause high memory usage on middleware servers, making them crash due to OOM.
- Exhaust bandwith on the database and middleware

This extension prevents those queries from running and transferring data.

It does so by aborting query execution at the planner level by using a `max_width` parameter that limits the `total_width`,
which is the `plan_width`(the average row size in bytes) times the `plan_rows`(the number of rows in the result set).

Note that the plan width is not exact since it doesn't consider the TOASTed([ref](https://wiki.postgresql.org/wiki/TOAST)) rows size.
So some result sets can be very wide but the plan won't show their total size.

```sql
-- example of the above

create table wide (id int, content text);

insert into wide select x, repeat('x', 10000) from generate_series(1, 10) x;
analyze wide;

explain table wide;
┌───────────────────────────────────────────────────────┐
│                      QUERY PLAN                       │
├───────────────────────────────────────────────────────┤
│ Seq Scan on wide  (cost=0.00..1.10 rows=10 width=129) │
└───────────────────────────────────────────────────────┘

select avg(octet_length(content)) from wide;
┌────────────────────────┐
│          avg           │
├────────────────────────┤
│ 10000.0000000000000000 │
└────────────────────────┘
-- planner width is 129 while the actual average byte size is 10000
```

### Example Usage

```sql
-- set a limit and reload the conf with a superuser
alter system set width_filter.max_width TO 1000;
select pg_reload_conf();

-- try a query with a non-superuser
create role nosuper;
grant all on table wide to nosuper;
set role nosuper;

-- wide query
select * from wide;
ERROR:  54001: max width exceeded
DETAIL:  the query width of 1290.000000 exceeds the max width of 1000.000000

-- a less wide query will pass
select * from wide limit 7;
-- content omitted for brevity

-- of course if you don't select the wide columns then the query will be accepted
select id from wide;
┌────┐
│ id │
├────┤
│  1 │
│  2 │
│  3 │
│  4 │
│  5 │
│  6 │
│  7 │
└────┘
```

MODULE_big = width_filter
OBJS = src/width_filter.o

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

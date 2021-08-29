#include "postgres.h"

#include <float.h>
#include "miscadmin.h"

#include "optimizer/planner.h"
#include "utils/guc.h"

#define PG13_GTE (PG_VERSION_NUM >= 130000)

/*
 * The planner hook signature varies between versions,
 * define some macros to handle that. */

#if PG13_GTE
#define PLANNER_HOOK_PARAMS \
	Query *parse, const char *queryString \
, int cursorOptions, ParamListInfo boundParams
#else
#define PLANNER_HOOK_PARAMS \
	Query *parse \
, int cursorOptions, ParamListInfo boundParams
#endif

#if PG13_GTE
#define PLANNER_HOOK_ARGS \
	parse, queryString \
, cursorOptions, boundParams
#else
#define PLANNER_HOOK_ARGS \
	parse \
, cursorOptions, boundParams
#endif

#define DEFAULT_MAX_WIDTH 100000.0

PG_MODULE_MAGIC;

static double max_width = DEFAULT_MAX_WIDTH;

static planner_hook_type prev_hook = NULL;

static PlannedStmt *
width_filter(PLANNER_HOOK_PARAMS);

void		_PG_init(void);
void		_PG_fini(void);

void
_PG_init(void)
{
	DefineCustomRealVariable("width_filter.max_width",
							""
							"",
							"",
							&max_width,
							DEFAULT_MAX_WIDTH,
							0.0, DBL_MAX,
							PGC_SUSET,
							0,
							NULL,
							NULL,
							NULL);

	prev_hook = planner_hook;
	planner_hook = width_filter;
}

void
_PG_fini(void)
{
	planner_hook = prev_hook;
}

static PlannedStmt *
width_filter(PLANNER_HOOK_PARAMS)
{
	PlannedStmt *result;
	double total_width;

	if (prev_hook)
		result = prev_hook(PLANNER_HOOK_ARGS);
	else
		result = standard_planner(PLANNER_HOOK_ARGS);

	if (!superuser()){
		total_width = result->planTree->plan_rows * result->planTree->plan_width;
		if (total_width > max_width)
			ereport(ERROR,
					(errcode(ERRCODE_STATEMENT_TOO_COMPLEX),
					 errmsg("max width exceeded"),
					 errdetail("the query width of %f exceeds the max width of %f", total_width, max_width)
					 ));
	}

	return result;
}

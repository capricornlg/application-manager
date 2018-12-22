#ifndef APPLICATION_DEFINITION_PERIOD_RUN_H
#define APPLICATION_DEFINITION_PERIOD_RUN_H

#include "ApplicationShortRun.h"

//////////////////////////////////////////////////////////////////////////
// Application that will start periodly but keep running all the time
//////////////////////////////////////////////////////////////////////////
class ApplicationPeriodRun :public ApplicationShortRun
{
public:
	ApplicationPeriodRun();
	virtual ~ApplicationPeriodRun();

	static void FromJson(std::shared_ptr<ApplicationPeriodRun>& app, const web::json::object& jobj);
	virtual web::json::value AsJson(bool returnRuntimeInfo) override;

	virtual void refreshPid() override;

	virtual void dump() override;
};

#endif
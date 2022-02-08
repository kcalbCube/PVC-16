#pragma once

#define ENABLE_EXECUTION_TIME_CAPTURE
#define ENABLE_WORKFLOW

inline struct VMFlags
{
#ifdef ENABLE_WORKFLOW
	bool workflowEnabled = false;
#endif
	int loadOffset = 0;
#ifdef ENABLE_EXECUTION_TIME_CAPTURE
	bool captureExecutionTime = false;
#endif
} vmflags;
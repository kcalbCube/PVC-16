#pragma once

#define ENABLE_EXECUTION_TIME_CAPTURE
#define ENABLE_WORKFLOW
#define ENABLE_OPCODE_STATISTICS
// TODO: remove this
#ifndef DISABLE_VIDEO
#define ENABLE_VIDEO
#endif
inline struct VMFlags
{
#ifdef ENABLE_WORKFLOW
	bool workflowEnabled = false;
#endif
	int loadOffset = 0;
#ifdef ENABLE_EXECUTION_TIME_CAPTURE
	bool captureExecutionTime = false;
#endif
#ifdef ENABLE_OPCODE_STATISTICS
	bool captureOpcodeStatistics = false;
#endif
} vmflags;
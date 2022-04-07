#pragma once

#define ENABLE_EXECUTION_TIME_CAPTURE

/*
 * --dworkflow
 */
//#define ENABLE_WORKFLOW

//#define ENABLE_OPCODE_STATISTICS

/*
 * TODO:
 */
//#define ENABLE_MEMORY_ACCESS_CHECK

/*
 * print debug message on invalid opcode and cont instead of crashing.
 */
//#define INVALID_OPCODE_SAFE

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
#ifdef ENABLE_OPCODE_STATISTICS
	bool captureOpcodeStatistics = false;
#endif
} vmflags;

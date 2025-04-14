// GLOBALS_H

#pragma once

extern JobIDs IDs;
extern set<Job, JobCompare> jobList;
extern unordered_map<string, function<void(const Command&)>> internalCmds;
extern string prevWd;
extern pid_t smashPID;
extern pid_t currTaskPid;
extern volatile sig_atomic_t sentSigint;
extern volatile sig_atomic_t sentSigtstp;
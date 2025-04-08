// GLOBALS_H

#pragma once

extern JobIDs IDs;
extern set<Job, JobCompare> jobList;
extern unordered_map<string, function<void(const Command&)>> internalCmds;
extern string prevWd;
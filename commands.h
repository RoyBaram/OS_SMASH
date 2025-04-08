#ifndef COMMANDS_H
#define COMMANDS_H
/*=============================================================================
* includes, defines, usings
=============================================================================*/
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>
#include <set>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

#define CMD_LENGTH_MAX 80
#define ARGS_NUM_MAX 20
#define JOBS_NUM_MAX 100
#define NO_ID -1

/*=============================================================================
* error handling - some useful macros and examples of error handling,
* feel free to not use any of this
=============================================================================*/
#define ERROR_EXIT(msg) \
    do { \
        cerr << __FILE__ << ": " << __LINE__ << \
        endl << msg; \
        exit(1); \
    } while(0);

/*=============================================================================
* classes/structs declarations
=============================================================================*/

typedef enum {
    FOREGROUND = 0,
    BACKGROUND,
    STOPPED
} JobStatus;

class Command {
private:
    bool bg;
    string cmd;
    vector<string> args;

public:
    Command(bool bg = false, const string& cmd = "", const vector<string>& args = {})
        : bg(bg), cmd(cmd), args(args) {}

    void setCmd(const string& newCmd) {
        cmd = newCmd;
    }

    string getCmd() const {
        return cmd;
    }

    void setBg(bool bgCond) {
        bg = bgCond;
    }

    bool isBg() const {
        return bg;
    }

    const vector<string>& getArgs() const {
        return args;
    }

    void setArgs(const vector<string>& newArgs) {
        args = newArgs;
    }
};

class Job : public Command {
private:
    int jobID;
    pid_t pid;
    JobStatus status;

public:
    Job() : Command(), jobID(NO_ID), pid(NO_ID) {}
    Job(int ID) : Command(), jobID(ID), pid(NO_ID) {}
    Job(int ID, Command cmd) : Command(cmd), jobID(ID), pid(NO_ID) {}

    int getJobID() const {
        return jobID;
    }

    void setJobID(int newID) {
        jobID = newID;
    }

    JobStatus getStatus() const {
        return status;
    }

    void setStatus(JobStatus newStatus) {
        status = newStatus;
    }

    pid_t getJobPid() const {
        return pid;
    }

    void setJobPid(pid_t newPid) {
        pid = newPid;
    }
};

class JobIDs {
private:
    set<int> available;

public:
    JobIDs() {
        for (int i = 0; i < JOBS_NUM_MAX; i++) {
            available.insert(i);
        }
    }

    int getID() {
        int id = *available.begin();
        available.erase(id);
        return id;
    }

    void insertID(int id) {
        available.insert(id);
    }

    bool isEmpty() {
        return available.empty();
    }

    size_t getSize() {
        return available.size();
    }
};

struct JobCompare {
    bool operator()(const Job& a, const Job& b) const {
        return a.getJobID() < b.getJobID();
    }
};

/*=============================================================================
* error definitions, enums
=============================================================================*/
typedef enum  {
	INVALID_COMMAND = 1,
	//feel free to add more values here or delete this
} ParsingError;

typedef enum {
	SMASH_SUCCESS = 0,
	SMASH_QUIT,
	SMASH_FAIL
	//feel free to add more values here or delete this
} CommandResult;

/*=============================================================================
* global functions
=============================================================================*/
int parseCmd(const string& line, Command& result);
void checkJoblStatus(set<Job> jobList);

#endif //COMMANDS_H
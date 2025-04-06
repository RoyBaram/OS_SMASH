#ifndef COMMANDS_H
#define COMMANDS_H
/*=============================================================================
* includes, defines, usings
=============================================================================*/
#include <cstdlib.h>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

#define CMD_LENGTH_MAX 80
#define ARGS_NUM_MAX 20
#define JOBS_NUM_MAX 100
#define NO_JOB_ID -1

/*=============================================================================
* error handling - some useful macros and examples of error handling,
* feel free to not use any of this
=============================================================================*/
#define ERROR_EXIT(msg) \
    do { \
        cerr << __FILE__ << ": " << __LINE__ << \
        endl << msg; \
        // fprintf(stderr, "%s: %d\n%s", __FILE__, __LINE__, msg); \ // C syntax
        exit(1); \
    } while(0);

/*=============================================================================
* classes/structs declarations
=============================================================================*/

class Command {
private:
    bool bg;
    string cmd;
    vector<string> args;

public:
    Command(bool status = false, const string& cmd = "", const vector<string>& args = {})
        : bg(status), cmd(cmd), args(args) {}

    Command() : bg(false), cmd(), args() {}

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

}

class Job : public Command {
private:
    int jobID;
    Command command;

public:
    Job() : jobID(NO_JOB_ID), command() {}
    Job(int ID) : jobID(ID), command() {}
    Job(int ID, Command cmd) : jobID(ID), command(cmd) {}

    int getJobID() const {
        return jobID;
    }

    void setJobID(int newID) {
        jobID = newID;
    }

    const Command& getCommand() const {
        return command;
    }

    void setCommand(const Command& cmd) {
        command = cmd;
    }

}


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

typedef enum {
    FOREGROUND = 0,
    BACKGROUND,
    STOPPED
} JobStatus;

/*=============================================================================
* global functions
=============================================================================*/
int parseCmd(string line, CmdArgs& result);

#endif //COMMANDS_H
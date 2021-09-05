#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Simplifed xv6 shell.

#define MAXARGS 10

// All commands have at least a type. Have looked at the type, the code
// typically casts the *cmd to some specific cmd type.
struct cmd {
    int type; //  ' ' (exec), | (pipe), '<' or '>' for redirection
};

struct execcmd {
    int type;            // ' '
    char *argv[MAXARGS]; // arguments to the command to be exec-ed
};

struct sequencecmd {
    int type;         // ;
    struct cmd *cmd;  // the command to be run
    struct cmd *next; // next cmd to be run
};

struct parallelcmd {
    int type;         // &
    struct cmd *cmd;  // the command to be run
    struct cmd *next; // next cmd to be run in parallel
};

int fork1(void); // Fork but exits on failure.
struct cmd *parsecmd(char *);

// Execute cmd.  Never returns.
void runcmd(struct cmd *cmd) {
    int p[2], r;
    struct execcmd *ecmd;
    struct sequencecmd *scmd;
    struct parallelcmd *prcmd;

    if (cmd == 0)
        exit(0);

    switch (cmd->type) {
    default:
        fprintf(stderr, "Invalid Syntax\n");
        exit(-1);
    case ' ':
        ecmd = (struct execcmd *)cmd;
        if (execvp(ecmd->argv[0], ecmd->argv) < 0) // Execute command, printing error if not found
            printf("%s: Command not found\n", ecmd->argv[0]);
        break;
    case ';':
        scmd = (struct sequencecmd *)cmd;
        if (fork() == 0)
            runcmd(scmd->cmd);
        wait(&r);
        runcmd(scmd->next);
        break;
    case '&':
        prcmd = (struct parallelcmd *)cmd;
        while (prcmd != NULL) {
            if (fork() == 0)
                runcmd(prcmd->cmd);
            prcmd = (struct parallelcmd *)prcmd->next;
        }
        while (1) { // wait for all child processes to finish
            int status;
            if (wait(&status) == -1) {
                break; // no more child processes
            }
        }
        break;
    }
    exit(0);
}

int getcmd(char *buf, int nbuf) {
    memset(buf, 0, nbuf);
    fgets(buf, nbuf, stdin);
    if (buf[0] == 0) // EOF
        return -1;
    return 0;
}

/**
 * @brief Main function of shell
 * 
 * @return int
 */
int main() {
    static char cmdBuf[100];
    int r;

    int emit_prompt = isatty(fileno(stdin));

    // Read and run input commands
    while (1) {
        if (emit_prompt)
            fprintf(stdout, "$CS450 ");

        getcmd(cmdBuf, sizeof(cmdBuf));

        if (fork() == 0)
            runcmd(parsecmd(cmdBuf));

        wait(&r);
    }

    exit(0);
}

int fork1(void) {
    int pid;

    pid = fork();
    if (pid == -1)
        perror("fork");
    return pid;
}

struct cmd *execcmd(void) {
    struct execcmd *cmd;

    cmd = malloc(sizeof(*cmd));
    memset(cmd, 0, sizeof(*cmd));
    cmd->type = ' ';
    return (struct cmd *)cmd;
}

// Parsing

char whitespace[] = " \t\r\n\v";
char symbols[] = "<|>;&";

int gettoken(char **ps, char *es, char **q, char **eq) {
    char *s;
    int ret;

    s = *ps;
    while (s < es && strchr(whitespace, *s))
        s++;
    if (q)
        *q = s;
    ret = *s;
    switch (*s) {
    case 0:
        break;
    case ';':
    case '&':
        s++;
        break;
    default:
        ret = 'a';
        while (s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
            s++;
        break;
    }
    if (eq)
        *eq = s;

    while (s < es && strchr(whitespace, *s))
        s++;
    *ps = s;
    return ret;
}

int peek(char **ps, char *es, char *toks) {
    char *s;

    s = *ps;
    while (s < es && strchr(whitespace, *s))
        s++;
    *ps = s;
    return *s && strchr(toks, *s);
}

struct cmd *parseline(char **, char *);
struct cmd *parseexec(char **, char *);

// make a copy of the characters in the input buffer, starting from s through es.
// null-terminate the copy to make it a string.
char *mkcopy(char *s, char *es) {
    int n = es - s;
    char *c = malloc(n + 1);
    assert(c);
    strncpy(c, s, n);
    c[n] = 0;
    return c;
}

static int invalid = 0; // Whether the current cmd is invalid

struct cmd *parsecmd(char *s) {
    char *es;
    struct cmd *cmd;

    int len = strlen(s);
    es = s + len;

    invalid = peek(&s, es, "&"); // Invalid check

    cmd = parseline(&s, es);

    if (invalid) {
        invalid = 0;

        cmd = parseexec(&s, es);
        cmd->type = -1;
        return cmd;
    }

    peek(&s, es, "");
    if (s != es) {
        fprintf(stderr, "leftovers: %s\n", s);
        exit(-1);
    }
    return cmd;
}

struct cmd *parallelcmd(struct cmd *set_cmd, struct cmd *next) {
    struct parallelcmd *cmd;

    cmd = malloc(sizeof(*cmd));
    memset(cmd, 0, sizeof(*cmd));
    cmd->type = '&';
    cmd->cmd = set_cmd;
    cmd->next = next;
    return (struct cmd *)cmd;
}

struct cmd *sequencecmd(struct cmd *set_cmd, struct cmd *next) {
    struct sequencecmd *cmd;

    cmd = malloc(sizeof(*cmd));
    memset(cmd, 0, sizeof(*cmd));
    cmd->type = ';';
    cmd->cmd = set_cmd;
    cmd->next = next;
    return (struct cmd *)cmd;
}

struct cmd *parseparallel(char **ps, char *es) {
    struct cmd *cmd;
    static int expecting = 0; // parallel command is expecting another cmd

    // End of cmd reached when expecting
    if (expecting && **ps == 0) {
        invalid = 1;
        expecting = 0;
        return cmd;
    }

    cmd = parseexec(ps, es);

    int hasSymbol = peek(ps, es, "&");
    if (hasSymbol || expecting) {
        expecting = hasSymbol || !expecting;

        if (expecting) {
            gettoken(ps, es, 0, 0);
            // End of cmd reached when expecting
            if (peek(ps, es, ";")) {
                expecting = 0;
                invalid = 1;
                return cmd;
            }
        }
        cmd = parallelcmd(cmd, parseparallel(ps, es));
    }
    return cmd;
}

struct cmd *parsesequence(char **ps, char *es) {
    struct cmd *cmd;

    cmd = parseparallel(ps, es);

    if (peek(ps, es, ";")) {
        gettoken(ps, es, 0, 0);
        cmd = sequencecmd(cmd, parseline(ps, es));
    }

    return cmd;
}

struct cmd *parseline(char **ps, char *es) {
    struct cmd *cmd;
    cmd = parsesequence(ps, es);
    return cmd;
}

struct cmd *parseexec(char **ps, char *es) {
    char *q, *eq;
    int tok, argc;
    struct execcmd *cmd;
    struct cmd *ret;

    ret = execcmd();
    cmd = (struct execcmd *)ret;

    argc = 0;
    while (!peek(ps, es, ";&")) {
        if ((tok = gettoken(ps, es, &q, &eq)) == 0)
            break;
        if (tok != 'a') {
            fprintf(stderr, "syntax error\n");
            exit(-1);
        }
        cmd->argv[argc] = mkcopy(q, eq);
        argc++;
        if (argc >= MAXARGS) {
            fprintf(stderr, "too many args\n");
            exit(-1);
        }
    }
    cmd->argv[argc] = 0;
    return ret;
}
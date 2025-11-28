#ifndef CS_OVER_PC_H
#define CS_OVER_PC_H

#include <cstddef>
#include <cstdlib>

#ifdef linux
#define _API_CALL
#define DLLEXPORT extern "C"
#else  //win, rtx
#define _API_CALL __stdcall
#define DLLEXPORT extern "C" __declspec(dllexport)
#endif

typedef unsigned int    u32;
typedef char    u8;

#define	FB_SIZE 	10*1024
#define	RV_SIZE		10*1024

typedef struct{
    u32	fb_len;
    u8	fb_buf[FB_SIZE];
} fb_st;

int rfs_structinit();

int rfs_cmdsolved(char *buf, unsigned int feedback_type, char *feedback_buf = nullptr);

int set_target(unsigned int board);

unsigned int get_target();

class CsOverPc {
private:

    u8	 	*pg;
    fb_st	*fb;
    u32 *send_buf;
    u32 *recv_buf;
    u8	*send_char;
    unsigned int target_board = 0;
public:

    CsOverPc() {
        pg = nullptr;
        recv_buf = nullptr;
        send_buf = nullptr;
        send_char = nullptr;
        fb = nullptr;
        target_board = 0;
    }

    ~CsOverPc() {
        cleanup();
    }

    void cleanup() {
        if (pg) {
            free(pg);
            pg = nullptr;
        }
        if (recv_buf) {
            free(recv_buf);
            recv_buf = nullptr;
        }
        if (send_buf) {
            free(send_buf);
            send_buf = nullptr;
        }
        if (send_char) {
            free(send_char);
            send_char = nullptr;
        }
        if (fb) {
            free(fb);
            fb = nullptr;
        }
    }

    int rfs_structinit();

    int rfs_cmdsolved(unsigned int boardn, char *buf, unsigned int feedback_type, char *feedback_buf = nullptr);

    int set_target(unsigned int board);

    unsigned int get_target();
};

#endif
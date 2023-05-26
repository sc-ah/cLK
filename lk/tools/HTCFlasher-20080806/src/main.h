#define BUFSIZE		600000
#define RUUBLOCK	0x100000
#define READ_TIMEOUT 3
#define REC 1

extern int fd;
extern char buf[BUFSIZE];
extern char rec[REC];
extern int DEBUG;
extern int ZENITY;
extern char *device;

/* utils.c */
int control_c(int sig);
char* parseHTCBlock();
void zenity(int i);
void progress(int pc);
void hexdump(size_t len, size_t width);

/* io.c */
void opendev();
void get();
void send(const char *command);
void rsend(const char *command);
void fsend(const char *command);

/* prompt.c */
void prompt();

/* info.c */
void getinfo();

/* flasher.c */
void authBL();
int flashNBH(char *flashfile);

/* crc32.c */
unsigned long Crc32(const unsigned char *data, unsigned long size, unsigned long prev);

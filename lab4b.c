//NAME: Jai Vardhan Fatehpuria
//EMAIL: jaivardhan.f@gmail.com
//ID: 804817305

#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include <poll.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <ctype.h>
#include <mraa.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/errno.h>

int period = 1;
int scale = 'F';
int optlog = 0;
int stop = 0;

sig_atomic_t volatile run_flag = 1;
mraa_aio_context temp_sensor;
mraa_gpio_context button;

struct timeval current_time;
struct tm* time_info;

const int B = 4275;               // B value of the thermistor
const int R0 = 100000;            // R0 = 100k
char* log_file;
int log_fd;

void terminate()
{
	time_info = localtime(&current_time.tv_sec);
    run_flag=0;
	char report[100];
	sprintf(report, "%02d:%02d:%02d SHUTDOWN\n", time_info->tm_hour, time_info->tm_min, time_info->tm_sec);
	int writelength = strlen(report);
	if (write(STDOUT_FILENO, report, writelength) < 0)
	{
		fprintf(stderr, "Error: %s\n", strerror(errno));
		exit(1);
	}
	if (optlog)
	{
		if (write(log_fd, report, writelength) < 0)
		{
			fprintf(stderr, "Error: %s\n", strerror(errno));
			exit(1);
		}
	}
	exit(0);
}

void error_occured()
{
    fprintf(stderr, "Error has occured. \n");
    
}

float get_temp()
{
    int value = mraa_aio_read(temp_sensor);
    float R = 1023.0/value-1.0;
	R = R0*R;
	float ctemp = 1.0/(log(R/R0)/B+1/298.15)-273.15;
	if(scale=='F')
	{
		float ftemp = (ctemp*9)/5 + 32;
		return ftemp;
	}
	else
	{
		return ctemp;
	}
	
}

void printArray(int A[], int size) {
    int i;
    for (i=0; i < size; i++)
        printf("%d ", A[i]);
    printf("\n");
}


void exec_command(const char* command)
{
	if(optlog)
	{
		int writelength = strlen(command);
		if (write(log_fd, command, writelength) < 0)
		{
			fprintf(stderr, "Error: %s\n", strerror(errno));
			exit(1);
		}
		if (write(log_fd, "\n", 1) < 0)
		{
			fprintf(stderr, "Error: %s\n", strerror(errno));
			exit(1);
		}
	}
	if (strcmp(command, "SCALE=C") == 0)
	{
		scale = 'C';
	}
	else if (strcmp(command, "SCALE=F") == 0)
	{
		scale = 'F';
	}
	else if (strcmp(command, "STOP") == 0)
	{
		stop = 1;
	}
	else if (strcmp(command, "START") == 0)
	{
		stop = 0;
	}
	else if (strncmp(command, "LOG", 3) == 0)
	{

	}
	else if (strcmp(command, "OFF") == 0)
	{
		terminate();
	}
	else if (strncmp(command, "PERIOD=", 7) == 0)
	{
			period = atoi(command + 7);
	}
}

int main(int argc, char* argv[]) {


	struct option LongOps[] = {
		{"period", required_argument, NULL, 'p'},
		{"scale", required_argument, NULL, 's'},
		{"log", required_argument, NULL, 'l'},
		{0, 0, 0, 0}
	};

	int opt;
	while ((opt = getopt_long(argc, argv, "", LongOps, NULL)) != -1) {
		switch (opt) {
			case 's':
                if (optarg[0] == 'C')
                    scale = 'C';
                break;
            case 'p':
                period = atoi(optarg);
                break;
            case 'l':
                optlog = 1;
				log_file = optarg;
                break;
			default:
				fprintf(stderr, "Error: Please input valid argument");
				exit(1);
		}
	}


    temp_sensor = mraa_aio_init(1);
	button = mraa_gpio_init(60);

	mraa_gpio_dir(button, MRAA_GPIO_IN);
	mraa_gpio_isr(button, MRAA_GPIO_EDGE_RISING, &terminate, NULL);


	gettimeofday(&current_time, NULL);
	time_t next_time;
	next_time = current_time.tv_sec + period;



	if(optlog)
	{
		log_fd = creat(log_file, 0666);
		if(log_fd<0)
		{
			fprintf(stderr, "Log file not created, Error: %s\n", strerror(errno));
			exit(1);
		}
	}
	struct pollfd fds[1];
	fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

	char buffer[32];
	char command[32];
	int current = 0;

    while(run_flag) {
        gettimeofday(&current_time, NULL);
		if (current_time.tv_sec >= next_time)
		{
			if (!stop)
			{
				float temperature = get_temp();
				next_time = current_time.tv_sec + period;
				time_info = localtime(&current_time.tv_sec);
				char report[100];
				sprintf(report, "%02d:%02d:%02d %0.1f\n", time_info->tm_hour, time_info->tm_min, time_info->tm_sec, temperature);
				int writelength = strlen(report);
				if (write(STDOUT_FILENO, report, writelength) < 0)
				{
					fprintf(stderr, "Cannot write to stdout, Error: %s\n", strerror(errno));
					exit(1);
				}
				if (optlog)
				{
					if (write(log_fd, report, writelength) < 0)
					{
						fprintf(stderr, "Cannot write to logfile, Error: %s\n", strerror(errno));
						exit(1);
					}
				}
			}
		}
        
        int ret = poll(fds, 1, 0);


		if (ret<0)
		{
			fprintf(stderr, "Polling error: %s\n", strerror(errno));
			exit(1);
		}
        else if (ret>0)
        {
			if (fds[0].revents == POLLIN)
			{
				int read_length = read(STDIN_FILENO, &buffer, 100);
				if (read_length < 0)
				{
					fprintf(stderr, "Read error: %s\n", strerror(errno));
					exit(1);
				}
                int i = 0;
                while (while < read_length) {
                    if (buffer[i] == '\n')
                    {
                        command[current] = '\0';
                        exec_command(command);
                        current = 0;
                    }
                    else
                    {
                        command[current] = buffer[i];
                        current++;
                    }
                    i++;
                }
			}
		}
	}
    mraa_aio_close(temp_sensor);
	mraa_gpio_close(button);
    exit(0);
}

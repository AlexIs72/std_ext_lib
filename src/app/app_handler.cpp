#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/resource.h>

#include <iostream>

#include "config.h"

#include "../sys/logger.h"
#include "../types/typedefs.h"
#include "app_handler.h"

typedef struct {
    struct option opt;
    const char *descr;
} cmd_line_arg;

cmd_line_arg cmd_line_args[] = {
//    {"version", 'v',    "Print application version" },
    { {"version", no_argument,  0,  'v' }, "Print application version" },
    { {"help",    no_argument,  0,  'h' }, "Print this message" }
//    {"help",    'h',    "Print this message" }
};

void print_usage(const std::string &app_name) {

    std::cout << "Usage: " << app_name << " <option> " << std::endl;
    std::cout << "  where 'option' is: " << std::endl;
    std::cout << "    " << 
        "--" << cmd_line_args[0].opt.name << "|-" << (char)cmd_line_args[0].opt.val <<
        "\t" << cmd_line_args[0].descr << std::endl;
    std::cout << "    " << 
        "--" << cmd_line_args[1].opt.name << "|-" << (char)cmd_line_args[1].opt.val <<
        "\t" << cmd_line_args[1].descr << std::endl;
}


std::string basename(const char *path) {
    std::string s = path;
    char sep = '/';
#ifdef _WIN32
    sep = '\\';
#endif
    size_t i = s.rfind(sep, s.length());
    if (i != std::string::npos) 
    {
        std::string filename = s.substr(i+1, s.length() - i);
        size_t lastindex = filename.find_last_of("."); 
        std::string rawname = filename.substr(0, lastindex); 
        return(rawname);
    }

    return("");
}

volatile static int __exit_flag = 0;
static core_application *__app = nullptr;

app_handler::app_handler() :
    m_old_sigint(nullptr),
    m_old_sigtem(nullptr),
    m_old_sigabrt(nullptr),
    m_old_sigquit(nullptr)
{
    
}

app_handler::~app_handler()
{
    __app = nullptr;
}

// cppcheck-suppress unusedFunction
int app_handler::parse_args(int argc, char *argv[])
{
    int ret_value = 0;
//	int this_option_optind = optind ? optind : 1;
	int option_index = 0;
	static struct option long_options[] = {
//		{"version", no_argument,  0,  'v' },
//		{"help",  	no_argument,  0,  'h' },
        cmd_line_args[0].opt,
        cmd_line_args[1].opt,
		{0,         0,            0,  0 }
	};
    extern int opterr;
//    extern char *optarg;
//    extern int optind, opterr, optopt;

    opterr = 0;

    m_app_name = basename(argv[0]);

	while(1) {
		int c = getopt_long(argc, argv, "vh",
				long_options, &option_index);
//printf("===> c = %d; index = %d\n", c, option_index);
		if (c == -1)
			break;

		switch (c) {
			case 'v':
                print_version();
                ret_value = 1;    
				break;
			case 'h':
                print_usage(m_app_name);
                ret_value = 1;
				break;
			default:
                m_app_args.push_back(argv[optind-1]);
//				printf("?? getopt returned character code 0%o (%s)??\n", c, optarg);
//                printf(" ===>  optind = %d, optarg = %s\n", optind, argv[optind-1]);
		}
	}

    return ret_value;
}

void /*app_handler::*/exit_handler(int signal)
{
    UNUSED(signal);
    printf("  ===> signal: %d\n", signal);
    __exit_flag =1;
    if(__app)
    {

std::cout << "Call terminate" << std::endl;
        __app->terminate();
std::cout << "App terminated" << std::endl;
    }
}

static bool enable_core_dump(){
    struct rlimit corelim;

    corelim.rlim_cur = RLIM_INFINITY;
    corelim.rlim_max = RLIM_INFINITY;

    return (0 == setrlimit(RLIMIT_CORE, &corelim));
}


int app_handler::run(core_application &app)
{
//    const char *pid_dir = "/var/run";
//    int pid;
    int result = 0;
    
    enable_core_dump();

    print_version();

    __app = &app;

    if(m_app_lock.try_lock(m_app_name) != 0)
    {
//        logger::error(
        std::cout << "Application already running. Lock file: " 
                  <<  m_app_lock.get_lock_file().c_str() 
                  << std::endl;
        return -1;
    }

    if(m_app_pid.create(m_app_name) != 0)
    {
//        logger::error(
        std::cout << "PID file exists: "
                  <<  m_app_pid.get_pid_file().c_str()
                  <<  std::endl;
        return -1;
    }

    m_old_sigint = std::signal( SIGINT, & exit_handler );
    m_old_sigtem = std::signal( SIGTERM, & exit_handler );
    m_old_sigabrt = std::signal( SIGABRT, & exit_handler );
    m_old_sigquit = std::signal( SIGQUIT, & exit_handler );

//    for ( const auto s : { SIGINT, SIGTERM, SIGABRT, SIGQUIT } )
//    {
//        std::signal( s, & /*app_handler::*/exit_handler );
//    }

    if(app.attach(this) != 0)
    {
        std::cout << "Init of application failed" << std::endl;
        return -1;
    }

    result = app.run();

    std::signal( SIGINT, m_old_sigint); 
    std::signal( SIGTERM, m_old_sigtem); 
    std::signal( SIGABRT, m_old_sigabrt);
    std::signal( SIGQUIT, m_old_sigquit);

    app.detach(this);

//    while(m_exit_flag == 0)
//    {
//        usleep(500000);
//    }

//    fgetc(stdin);

//    UNUSED(app);
    return result;
}

void app_handler::print_version() const
{
    std::cout << 
        m_app_name << " " <<
        m_app_version << "(" << m_app_revision << ")" <<
        ", sdk " << SDK_VERSION << "(" << SDK_REVISION << ")" <<
        std::endl;
}

// cppcheck-suppress unusedFunction
void app_handler::set_app_version(const char *version, const char *revision)
{
    m_app_version = version;
    m_app_revision = revision;
}

/*
    struct sigaction sigact;
    sigset_t         sigset;
    int             signo;
    int             status;

    // сигналы об ошибках в программе будут обрататывать более тщательно
    // указываем что хотим получать расширенную информацию об ошибках
    sigact.sa_flags = SA_SIGINFO;
    // задаем функцию обработчик сигналов
    sigact.sa_sigaction = signal_error;

    sigemptyset(&sigact.sa_mask);

    // установим наш обработчик на сигналы
    
    sigaction(SIGFPE, &sigact, 0); // ошибка FPU
    sigaction(SIGILL, &sigact, 0); // ошибочная инструкция
    sigaction(SIGSEGV, &sigact, 0); // ошибка доступа к памяти
    sigaction(SIGBUS, &sigact, 0); // ошибка шины, при обращении к физической памяти

    sigemptyset(&sigset);
    
    // блокируем сигналы которые будем ожидать
    // сигнал остановки процесса пользователем
    sigaddset(&sigset, SIGQUIT);
    
    // сигнал для остановки процесса пользователем с терминала
    sigaddset(&sigset, SIGINT);
    
    // сигнал запроса завершения процесса
    sigaddset(&sigset, SIGTERM);
    
    // пользовательский сигнал который мы будем использовать для обновления конфига
    sigaddset(&sigset, SIGUSR1);
    sigprocmask(SIG_BLOCK, &sigset, NULL);

    // Установим максимальное кол-во дискрипторов которое можно открыть
    SetFdLimit(FD_LIMIT);
*/

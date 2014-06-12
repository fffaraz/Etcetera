#include <proc/readproc.h>
#include <proc/sysinfo.h>

float pid_age(pid_t pid)
{
        proc_t proc_info;
        int seconds_since_boot = uptime(0,0);
        if (!get_proc_stats(pid, &proc_info)) return 0.0;

        // readproc.h comment lies about what proc_t.start_time is. It's
        // actually expressed in Hertz ticks since boot

        int  seconds_since_1970 = time(NULL);
        int time_of_boot = seconds_since_1970 - seconds_since_boot;
        long  t = seconds_since_boot - (unsigned long)(proc_info.start_time / Hertz);

        int delta = t;
        float days = ((float) delta / (float)(60*60*24));
        return days;
}

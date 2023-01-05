#include <kernel.h>
#include <lib.h>
#include <stdio.h>
#include <name.h>
#include <semaphore.h>
#include <lfs.h>

/*------------------------------------------------------------------------
 * xsh_ls - shell command to copy stdin to a file and to stdout
 *------------------------------------------------------------------------
 */

shellcmd xsh_ls(int nargs, char *args[])
{
    if (nargs > 1 && strncmp(args[1], "--help", 7) == 0){
        printf("Usage: %s [-l] [dir]\n\n", args[0]);
        printf("Description:\n");
        printf("\tlist files in the default directory or\n");
        printf("\tthe directory specified by dir\n");
        printf("Options:\n");
        printf("\t-l\t list file size as well as name\n");
        printf("\t--help\t display this help and exit\n");
        return 0;
    }

    if (nargs > 3){
        fprintf(stderr, "Use: %s [-l] [dir]\n\n", args[0]);
        return 1;
    }

    int32 nextarg = 1;
    bool8 loption = FALSE;
    if (nargs > 1 && strncmp(args[nextarg], "-l", 3) == 0){
        loption = TRUE;
        nextarg++;
        nargs--;
    }

    char * fname; /* File name to map to a device */

    if (nargs > 1){
        if (args[nextarg][0] == '-'){
            fprintf(stderr, "Use: %s [-l] [dir]\n\n", args[0]);
            return 1;
        }
        fname = args[nextarg];
    }
    else{
        fname = ".";
    }
    char nbuf[NM_MAXLEN];
    did32 dev = namrepl(fname, nbuf);

    if (dev == (did32)SYSERR){
        fprintf(stderr, "%s: cannot access device for %s\n", args[0], fname);
        return 1;
    }

    /* Handle local file system */

    int32 retval;
    struct lfdir dir;

    if (dev == LFILESYS){
        wait(Lf_data.lf_mutex);
        if(!Lf_data.lf_dirpresent){
            retval = read(Lf_data.lf_dskdev, (char *)&Lf_data.lf_dir, LF_AREA_DIR);
            if (retval == SYSERR){
                fprintf(stderr, "%s: cannot obtain directory\n", args[0]);
                signal(Lf_data.lf_mutex);
                return 1;
            }
            if(lfscheck(&Lf_data.lf_dir) == SYSERR){
                fprintf(stderr, "%s: file system uninitialized\n", args[0]);
                signal(Lf_data.lf_mutex);
                return 1;    
            }
        }
        signal(Lf_data.lf_mutex);
        memcpy((void *)&dir, (void *)&Lf_data.lf_dir, sizeof(dir));
        for (int i = 0; i < dir.lfd_nfiles; i++){
            if (loption){
                fprintf(stdout, "%6d  %s\n", dir.lfd_files[i].ld_size,
                    dir.lfd_files[i].ld_name);
            }
            else{
                fprintf(stdout, "       %s\n", dir.lfd_files[i].ld_name);
            }
        }
    }
    else{ /* Handle erro case for unknown file system */
	    fprintf(stderr, "%s: file system is an unknown type\n",
		args[0]);
		return 1;    
    }
    return 0;
}

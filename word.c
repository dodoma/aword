/*
 * TODO
 * 1. 中英文同义词
 */
#include "reef.h"

struct aword {
    char eword[256];
    char cword[256];
    bool pass;
};

static void _pass_record(char *word)
{
    char command[1024] = {0};
    snprintf(command, sizeof(command), "egrep -sq '^%s\t[0-9]+' .passed", word);
    if (system(command) == 0) {
        //mtc_mm_dbg("main", "increment %s", word);
        snprintf(command, sizeof(command),
                 "awk -i inplace -F '\t' "
                 "'$1 ~ /%s/{$2=$2+1;print$1\"\t\"$2} $1 !~ /%s/{print}' .passed", word, word);
        system(command);
    } else {
        //mtc_mm_dbg("main", "append %s", word);
        snprintf(command, sizeof(command), "echo '%s\t1' >> .passed", word);
        system(command);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("useage: %s wordfile\n", argv[0]);
        return 1;
    }

    MERR *err;

    MRE *reo = mre_init();
    err = mre_compile(reo, "- (\\C+?)\\s+([\\c]+)$");
    DIE_NOK(err);

    struct aword *w;
    MLIST *wlist;
    mlist_init(&wlist, free);

    mtc_mm_init("-", "main", MTC_DEBUG);

    /*
     * 0. 读取待考单词文件
     */
    FILE *fp = fopen(argv[1], "r");
    char line[1024];
    const char *ep, *sp;
    if (fp) {
        while (fgets(line, sizeof(line) - 1, fp) != NULL) {
            line[strcspn(line, "\n")] = 0;

            if (mre_match(reo, line, false)) {
                w = mos_calloc(1, sizeof(struct aword));
                w->pass = false;
                mre_sub_get(reo, 0, 1, &sp, &ep);
                memcpy(w->eword, sp, ep - sp);    /* TODO memory overflow */
                mstr_tolower(w->eword);

                mre_sub_get(reo, 0, 2, &sp, &ep);
                memcpy(w->cword, sp, ep - sp);
                mstr_tolower(w->cword);

                mlist_append(wlist, w);
            }
        }

        fclose(fp);
    } else mtc_mm_err("main", "open word file %s failure", argv[1]);

    /*
     * 1. 过滤已熟练单词
     */
    MLIST_ITERATE(wlist, w) {
        char command[1024] = {0};
        snprintf(command, sizeof(command), "egrep -sq '^%s\t[3-9]$' .passed", w->eword);
        if (system(command) == 0) {
            mtc_mm_dbg("main", "filter passed word: %s", w->eword);
            mlist_delete(wlist, _moon_i);
            _moon_i--;
        }
    }

    /*
     * 2. 开始考试
     */
    int count = mlist_length(wlist);
    int passed = 0;
    printf("\n%d 个单词待熟悉\n\n", count);
    while (passed < count) {
        uint32_t index = mos_rand(count);
        w = (struct aword*)mlist_getx(wlist, index);
        while (w && w->pass) {
            index++;
            if (index >= count) index = 0;

            w = (struct aword*)mlist_getx(wlist, index);
        }

        if (w) {
            /* 考考你吧 */
            char *src, *dst, answer[256];
            if (index % 2 == 0) {
                src = w->eword;
                dst = w->cword;
            } else {
                src = w->cword;
                dst = w->eword;
            }

            printf(MCOLOR_RESET"> %s\n", src);
            /*
             * https://stackoverflow.com/questions/1247989/how-do-you-allow-spaces-to-be-entered-using-scanf
             */
            //scanf("%s", answer);
            fgets(answer, sizeof(answer), stdin);
            answer[strcspn(answer, "\n")] = 0;
            mstr_tolower(answer);

            if (strlen(answer) > 0 && (!strcmp(answer, dst) || strstr(dst, answer) || strstr(answer, dst))) {
                if (strcmp(answer, dst)) {
                    printf(MCOLOR_YELLOW"%s\n", dst);
                }
                w->pass = true;
                passed++;
                _pass_record(w->eword);
            } else if (!strcmp(answer, "q")) {
                printf("%.2f%% passed, Bye\n", (float)passed / count * 100.0);
                return 0;
            } else printf(MCOLOR_RED"%s\n", dst);

            printf("\n");
        }
    }

    printf("Well Done, Game Over\n");

    mre_destroy(&reo);
    mlist_destroy(&wlist);

    return 0;
}

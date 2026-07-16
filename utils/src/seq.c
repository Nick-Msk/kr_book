/********************************************************************
                    SEQUENCE MODULE IMPLEMENTATION
********************************************************************/

#include "seq.h"

enum { SEQ_MAXCOUNT = 128, G_TECHPTINTPERLINE = 10 };        // for now

// == 0 means dropped or not init
static seqv_t        sequences[SEQ_MAXCOUNT] = {0};
static seqnum_t      sequencesptr = 0;

// ------------------------------------ Utilities ------------------------------------------


int                 findseq(void) {
    for (seqnum_t i = 0; i < sequencesptr; i++)
        if (sequences[i])   // found dropped
            return logsimpleret(i, "Found %d", i);
    return logsimpleret(-1, "Unable to find empty seq");
}

// ------------------------------------- API -----------------------------------------------

seqnum_t            initseq(void) {
    seqnum_t     num;
    if (sequencesptr >= SEQ_MAXCOUNT) {
        if ( (num = findseq() ) < 0)
            userraiseint(ERR_UNABLE_ALLOCATE_SEQ, "Unable to allocate sequence");   // probably need to be configurable in context
    } else
        num = sequencesptr++;
    sequences[num] = 1L;    // init
    return num;
}

void                dropseq(seqnum_t s) {
    if (! (s >= 0 && s < SEQ_MAXCOUNT) )
        userraiseint(ERR_OUT_OF_RANGE, "%d must be positive and < %d", s, SEQ_MAXCOUNT);
    sequences[s] = 0L;    // un init
}

seqv_t              currval(seqnum_t s) {
    if (! (s >= 0 && s < SEQ_MAXCOUNT) )
        userraiseint(ERR_OUT_OF_RANGE, "%d must be positive and < %d", s, SEQ_MAXCOUNT);
    return sequences[s];
}

seqv_t           nextval(seqnum_t s) {
    if (! (s >= 0 && s < SEQ_MAXCOUNT) )
        userraiseint(ERR_OUT_OF_RANGE, "%d must be positive and < %d", s, SEQ_MAXCOUNT);
    return ++sequences[s];
}

// ------------------------ Printers ---------------------------------------
int                 seq_techfprint(FILE *out) {
    int     cnt = 0;
    if (out) {
        int total = 0;
        fprintf(out, "SEQ_T: %d/%d [\n", sequencesptr, SEQ_MAXCOUNT);
        for (int i = 0; i < sequencesptr; i++) {
            if (sequences[i])
                fprintf(out, "%3d:%6lld\t", i, sequences[i]), total++;
        }
        fprintf(out, "], total allocated %d\n", total);
    }
    return cnt;
}

// ---------------------------------------- Testing ------------------------------------------
#ifdef SEQTESTING

#include "test.h"

// ------------------------- TEST sequence API ---------------------------------
static TestStatus
tf_sequence(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    test_sub("subtest %d: init single sequence", ++subnum);
    {
        seqnum_t s = initseq();
        test_validate(s >= 0, "Failed to init sequence (got %d)", s);
        dropseq(s);
    }

    test_sub("subtest %d: currval after init is 1", ++subnum);
    {
        seqnum_t s = initseq();
        seqv_t v = currval(s);
        test_validate(v == 1L, "currval after init should be 1, got %ld", (long)v);
        dropseq(s);
    }

    test_sub("subtest %d: nextval increments", ++subnum);
    {
        seqnum_t s = initseq();
        seqv_t v1 = nextval(s);
        seqv_t v2 = nextval(s);
        seqv_t cur = currval(s);
        test_validate(v1 == 2L && v2 == 3L && cur == 3L,
                      "nextval: v1=%ld, v2=%ld, cur=%ld (expected 2,3,3)",
                      (long)v1, (long)v2, (long)cur);
        dropseq(s);
    }

    test_sub("subtest %d: drop and reuse slot", ++subnum);
    {
        seqnum_t a = initseq();                     // a=0
        seqnum_t b = initseq();                     // b=1
        dropseq(a);
        seqnum_t c = initseq();                     // должно занять слот 0
        test_validate(c == 0, "Reused slot should be 0, got %d", c);
        dropseq(b);
        dropseq(c);
    }

    test_sub("subtest %d: init more than SEQ_MAXCOUNT raises", ++subnum);
    {
        // Займём все слоты (предполагается, что SEQ_MAXCOUNT достаточно мал для теста, иначе займём часть и проверим, что после превышения выбрасывается исключение)
        // Для надёжности сохраним номера, чтобы потом освободить
        seqnum_t used[SEQ_MAXCOUNT] = {0};
        int cnt = 0;
        // Заполняем все возможные слоты
        for (int i = 0; i < SEQ_MAXCOUNT; i++) {
            seqnum_t s = initseq();
            if (s < 0) break;
            used[cnt++] = s;
        }
        // Теперь попытка создать ещё один должна вызвать ошибку
        if (!try()) {
            seqnum_t extra = initseq();          // должно упасть
            test_validate(false, "Should have raised SIGINT when out of slots");
            // если не упало, удаляем лишний
            if (extra >= 0) dropseq(extra);
        } else {
            logsimple("Exception correctly raised on sequence exhaustion");
        }
        // Освобождаем занятые слоты
        for (int i = 0; i < cnt; i++) {
            dropseq(used[i]);
        }
    }

    test_sub("subtest %d: drop invalid index raises", ++subnum);
    {
        if (!try()) {
            dropseq(-1);
            test_validate(false, "Should have raised on negative index");
        } else {
            logsimple("Exception on dropseq(-1)");
        }
        if (!try()) {
            dropseq(SEQ_MAXCOUNT);
            test_validate(false, "Should have raised on index == SEQ_MAXCOUNT");
        } else {
            logsimple("Exception on dropseq(SEQ_MAXCOUNT)");
        }
    }

    test_sub("subtest %d: currval/nextval invalid index raises", ++subnum);
    {
        if (!try()) {
            currval(-5);
            test_validate(false, "currval(-5) should have raised");
        } else {
            logsimple("Exception on currval(-5)");
        }
        if (!try()) {
            nextval(SEQ_MAXCOUNT + 10);
            test_validate(false, "nextval(out of range) should have raised");
        } else {
            logsimple("Exception on nextval(out of range)");
        }
    }
    test_sub("subtest %d: initseq reuses dropped slot when full", ++subnum);
    {
        // предполагаем, что SEQ_MAXCOUNT достаточно мал для теста (например, 4)
        seqnum_t used[SEQ_MAXCOUNT] = {0};
        int cnt = 0;

        // 1. Заполняем все слоты до максимума
        for (int i = 0; i < SEQ_MAXCOUNT; i++) {
            seqnum_t s = initseq();
            if (s < 0) break;
            used[cnt++] = s;
        }
        test_validate(cnt == SEQ_MAXCOUNT,
                      "Should have allocated all %d slots, got %d", SEQ_MAXCOUNT, cnt);

        // 2. Освобождаем слот с индексом 2 (не последний)
        dropseq(used[2]);

        // 3. Ещё раз запрашиваем последовательность – должен вернуться тот же индекс
        seqnum_t s = initseq();
        test_validate(s == used[2],
                      "initseq should reuse dropped index %d, got %d", used[2], s);
        test_validate(currval(s) == 1L,
                      "Reused sequence must start at 1, got %ld", (long)currval(s));

        // 4. Проверяем, что больше свободных слотов нет – следующая попытка должна вызвать ошибку
        if (!try()) {
            seqnum_t extra = initseq();
            test_validate(false, "Should have raised when no slots available");
            if (extra >= 0) dropseq(extra);
        } else {
            logsimple("Exception correctly raised after reusing last free slot");
        }

        // 5. Освобождаем все занятые слоты (кроме уже удалённого used[2])
        for (int i = 0; i < cnt; i++) {
            if (i != 2) dropseq(used[i]);
        }
        dropseq(s); // освобождаем повторно использованный
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------------------------------------------------------------------------------------------------------------
int
main(int argc, const char *argv[])
{
    logsimpleinit("Start");
    bool    runall = argc == 1;

    while (runall || *++argv){
        int     num = INT_MAX;    // INT_MAX for all test
        if (!runall){
            num = atoi(*argv);
            if (num < 0){
                fprintf(stderr,"Invalid test num %d\n", num);
                continue;
            }
        }
        testenginestd_run(num,
            testnew(.f2 = tf_sequence,                                 .num = 1, .name = "tf_value64_tarray"
                , .desc="", .mandatory=true)
        );
        if (runall)
            break;
    }

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* SEQTESTING */



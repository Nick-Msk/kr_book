/********************************************************************
                    HASH-BASED SET MODULE IMPLEMENTATION
********************************************************************/

#include "hashset.h"

// TODO: move that to context
static int                              HSET_ARRAY_CREATE_MULTIPLIER = 2;
static int                              HSET_MIN_SIZE                = 8;
static double                           HSET_NORMALIZE_FACTOR        = 1.5;
/*
static const size_t                     hset_elem_sizes[] = {
    [VALUE64_INT]  = sizeof(int),
    [VALUE64_LNG] = sizeof(long),
    [VALUE64_DBL]  = sizeof(double),
    [VALUE64_PTR]  = sizeof(void*),
    [VALUE64_FS]   = sizeof(fs *),
    [
};*/

// ---------- pseudo-header for utility procedures -----------------

// ---------------------------- (Utility) printers -----------------------------
// generic
static int                 sortedlist_fprint(FILE *restrict out, const hset_elem *restrict elem, value64_type typ){
    invraisecode(ERR_NULLABLE_PTR, out != NULL, "Null pointer");
    int     cnt = 0;
    while (elem){
        cnt += value64_fprint(out, elem->v, typ);
        /*switch (typ){
            case VALUE64_INT:
                cnt += fprintf(out, "%d", elem->v.ival);
            break;
            case VALUE64_LNG:
                cnt += fprintf(out, "%ld", elem->v.lval);
            break;
            case VALUE64_DBL:
                cnt += fprintf(out, "%lf", elem->v.dval);
            break;
            case VALUE64_PTR:
                cnt += fprintf(out, "%p", elem->v.pval);
            break;
            case VALUE64_FS:
                cnt += fs_fprint(out, elem->v.fsval, NULL);
            break;
            default:
                logsimple("Unsuported %d - %s", typ, value64_typename(typ) );
            break;
        }*/
        cnt += fprintf(out, " -> ");
        elem = elem->next;
    }
    return cnt;
}

// ------------------------------------ Utilities ------------------------------------------
// generic
static unsigned long       get_lhash(unsigned cnt, value64 value, value64_type typ){
    // probably it's better to calc hash by u64 attr (except fs for sure)
    unsigned long hash = value64_lhash(value, typ);
    return hash % cnt;
    /*
    hset_value      tmp = HSET_ZERO_VALUE;
    switch (typ){
        case VALUE64_INT:
            tmp.u64 = (uint64_t) value.ival;
        break;
        case VALUE64_LNG:
            tmp.u64 = (uint64_t) value.lval;
        break;
        case VALUE64_DBL:
            //bits = (uint64_t) value.lval;   // OMG
            tmp.u64 = value.u64;
        break;
        case VALUE64_PTR:
            tmp.u64 = (uint64_t)(uintptr_t) value.pval;    // or just do nothing as for VALUE64_DBL
        break;
        case VALUE64_FS:
            return hash_djb2(fs_str(value.fsval) ) % cnt;
        break;
        default:
        break;
    }
    return tmp.u64 % cnt; */
}
/*
static inline int           compare(hset_value v1, hset_value v2, value64_type typ){
    switch (typ){
        case VALUE64_INT:
            return compare_int(v1.ival, v2.ival);
        break;
        case VALUE64_LNG:
            return compare_long(v1.lval, v2.lval);
        break;
        case VALUE64_DBL:
            return compare_dbl(v1.dval, v2.dval);
        break;
        case VALUE64_PTR:
            return compare_ptr(v1.pval, v2.pval);
        break;
        case VALUE64_FS:
            return fs_cmp(v1.fsval, v2.fsval);
        break;
        default:
            userraiseint(ERR_UNSUPPORTED_TYPE, "%s: %d", value64_typename(typ), typ);
            return 0;
    }
} 

static value64_type            gettype(const char *str){
    return value64_gettype(str);
    if (strcmp(str, "VALUE64_INT") == 0)
        return  VALUE64_INT;
    else if (strcmp(str, "VALUE64_LNG") == 0)
        return  VALUE64_LNG;
    else if (strcmp(str, "VALUE64_DBL") == 0)
        return  VALUE64_DBL;
    else if (strcmp(str, "VALUE64_PTR") == 0)
        return  VALUE64_PTR;
    else if (strcmp(str, "VALUE64_FS") == 0)
        return VALUE64_FS;
    else
        return HSET_UKNOWN; 
}*/
/*
static hset_value           convert_value(hset_value v, value64_type from, value64_type to){
    if (from == to && from != VALUE64_FS)  // криво конечно
        return v;

    hset_value result = HSET_ZERO_VALUE; // обнуляем u64

    switch (from) {
        case VALUE64_INT:
            if (to == VALUE64_LNG)
                result.lval = (long)v.ival;
            else if (to == VALUE64_DBL)
                result.dval = (double)v.ival;
            else if (to == VALUE64_FS) {
                fs tmp = FS();
                fs_sprintf(&tmp, "%d", v.ival);
                result.fsval = fs_moveto_heap(&tmp);
            } else
                userraiseint(ERR_UNSUPPORTED_TYPE_CONV, "from %d:%s to %d:%s", from, value64_typename(from), to, value64_typename(to) );
        break;
        case VALUE64_LNG:
            if (to == VALUE64_INT)
                result.ival = (int)v.lval;
            else if (to == VALUE64_DBL)
                result.dval = (double)v.lval;
            else if (to == VALUE64_FS) {
                    fs tmp = FS();
                    fs_sprintf(&tmp, "%ld", v.lval);
                    result.fsval = fs_moveto_heap(&tmp);
            } else
                userraiseint(ERR_UNSUPPORTED_TYPE_CONV, "from %d:%s to %d:%s", from, value64_typename(from), to, value64_typename(to) );
            break;
        case VALUE64_DBL:
            if (to == VALUE64_INT)
                result.ival = (int)v.dval;
            else if (to == VALUE64_LNG)
                result.lval = (long)v.dval;
            else if (to == VALUE64_FS) {
                    fs tmp = FS();
                    fs_sprintf(&tmp, "%g", v.dval); // TODO: context must be here!
                    result.fsval = fs_moveto_heap(&tmp);
            } else
                userraiseint(ERR_UNSUPPORTED_TYPE_CONV, "from %d:%s to %d:%s", from, value64_typename(from), to, value64_typename(to) );
            break;
        case VALUE64_FS:
            if (to == VALUE64_INT)
                result.ival = fs_getint(v.fsval);
            else if (to == VALUE64_LNG)
                result.lval = fs_getlong(v.fsval);
            else if (to == VALUE64_DBL)
                result.dval = fs_getdouble(v.fsval);
            else if (to == VALUE64_FS)
                result.fsval = fs_heapcreate(v.fsval); // body in heap too!!!
            else
                userraiseint(ERR_UNSUPPORTED_TYPE_CONV, "from %d:%s to %d:%s", from, value64_typename(from), to, value64_typename(to) );
            break;
        default:
            break; // неподдерживаемые типы — останется нулевое значение
    }
    return result;
}

static void                 printval(FILE *out, value64_type typ, value64 v){
    if (out){
        switch (typ) {
            case VALUE64_INT:
                fprintf(out, "%d\n", v.ival);
            break;
            case VALUE64_LNG:
                fprintf(out, "%ld\n", v.lval);
            break;
            case VALUE64_DBL:
                fprintf(out, "%.15g\n", v.dval);
            break;
            case VALUE64_PTR:
                fprintf(out, "%p\n", v.pval);
            break;
            case VALUE64_FS:
                fs_fprint(out, v.fsval, NULL); fprintf(out, "\n");  // TODO: ref is required
            break;
            default:
                break;
        }
    }
} 

static bool                 readval(FILE *f, value64_type typ, value64 *v) {
    switch (typ) {
        case VALUE64_INT:
            return fscanf(f, "%d", &v->ival) == 1;
        case VALUE64_LNG:
            return fscanf(f, "%ld", &v->lval) == 1;
        case VALUE64_DBL:
            return fscanf(f, "%lf", &v->dval) == 1;
        case VALUE64_PTR:
            return fscanf(f, "%p", &v->pval) == 1;
        case VALUE64_FS:
            v->fsval = fs_fscanf(f, NULL); // TODO: ref is required to make smth like return fs_fscanf(f, &v->fsval) == true;
            return v->fsval != NULL;
        default: return false;
    }
} */

static inline value64_type     getype(const hset *se){
    return se->flags & 0xFF;
}
// seach the value in hset!
static hset_elem           *getprevelem(const hset *restrict se, value64 value,
        unsigned *restrict phash, hset_elem **restrict pnext, hset_elem **restrict pequal){

    unsigned hash = get_lhash(se->sz, value, getype(se) );
    hset_elem *el = se->table[hash],
              *prevel = 0;
    while (el != 0 && value64_compare(value, el->v, getype(se) ) < 0){
        //logmsg("el %p, prevel %p", el, prevel);
        prevel = el;
        el = el->next;
    }
    // logmsg("after: el %p", el);
    if (phash)
        *phash = hash;
    if (pnext)
        *pnext = el;
    if (pequal){
        if (el && value64_compare(el->v, value, getype(se) ) == 0) //  found EXACLTY!
            *pequal = el;
        else
            *pequal = 0;        // NOT found exactly!
    }
    return prevel; //logsimpleret(prevel, "Found prev %p, next %p", prevel, el);   // < or = in the list
}

static hset_elem           *alloc_elem(void){
    return malloc(sizeof(hset_elem) );
}

static hset_elem           *create_elem(value64 val, value64_type typ){
    hset_elem *res = alloc_elem();
    if (!res)
        return logsimpleret( (hset_elem *) 0, "Unable to create elem");

    if (typ == VALUE64_FS) {    // TODO: not sure about  that solution
        if (val.fsval && !fs_bodyalloc(val.fsval))
            val.fsval = fs_heapcreate(val.fsval);
    }
    res->v = val;
    res->next = 0;

    return res;
}

static bool                 create_or_move_elem(hset * restrict se, hset_elem *restrict el, value64 val){
    if (getype(se) == VALUE64_FS)
        logsimple("%p %p", val.fsval, val.fsval ? fs_str(val.fsval) : NULL);
    if (getype(se) == VALUE64_FS && (val.fsval == NULL || fs_str(val.fsval) == NULL) )
        return logsimpleret(false, "Unable to add Null FS value");

    bool         already_existed = false;
    unsigned     hash = 0;
    hset_elem   *equal = 0, *nextel = 0;
    value64      value;
    if (el)
        value = el->v;
    else {
        // TODO: rework, shouldn't create and then free in (equal)
        //if (getype(se) == VALUE64_FS && !fs_bodyalloc(val.fsval) ){
          //  val.fsval = fs_heapcreate(val.fsval);    //  clone here!
        //}
        value = val;
    }
    hset_elem   *prevel = getprevelem(se, value, &hash, &nextel, &equal);
    if (equal) {
        already_existed = true;
        // TODO: not sure about that sln
        //if (getype(se) == VALUE64_FS){
        //    logsimple("DUPLICATE FS, freeing %p [%s]", val.fsval, val.fsval ? val.fsval->v : "");
        //    value64freefs(val);
        //}
    }
    else {
        hset_elem *newel;
        if (el)     // move
            newel = el;
        else {          // create a new one
            newel = create_elem(val, getype(se) );
            if (!newel)
                userraiseint(ERR_UNABLE_ALLOCATE, "Can't create new element");
        }
        if (prevel)
            prevel->next = newel;
        else
            se->table[hash] = newel;
        newel->next = nextel;
        se->count++;
    }
    return logsimpleret(!already_existed, "Added the value prev existing %s", bool_str(already_existed) );
}

static hset_elem           *clone_elemlist(const hset_elem *el, value64_type typ){

    hset_elem  *prev = 0, *newel = 0, *retel = 0;
    while (el){
        if (!(newel = create_elem(el->v, typ) ) )
            return userraise((hset_elem *) 0, ERR_UNABLE_ALLOCATE, "Unable to create element");
        switch (typ){
            case VALUE64_FS:
                newel->v.fsval = fs_heapcreate(el->v.fsval);    // TODO: not sure
            break;
            /* case HSET_STR:
                newel->v.str = strdup(el->v.str);
            break;*/
            default:
                newel->v = el->v;
            break;
        }
        if (!retel)
            retel = newel;  //  first elem
        if (prev)
            prev->next = newel;
        // next iter
        prev = newel;
        el = el->next;
    }
    return logsimpleret(retel, "Chain is created %p", retel);
}

static inline int           count_elemlist(const hset_elem *el){
    int     cnt = 0;
    while (el){
        cnt++;
        el = el->next;
    }
    return cnt;
}
static int                   hset_calc_cnt(const hset *se){
    invraise(se != 0, "Null pointer");

    int     cnt = 0;
    for (int i = 0; i < se->sz; i++)
        if (se->table[i]){  // actually this check is excess
            cnt += count_elemlist(se->table[i] );
        }
    return logsimpleret(cnt, "Total %d", cnt);
}

static void                 free_elem(hset_elem *el, value64_type typ){
    switch (typ){
        case VALUE64_FS:
            fs_free(el->v.fsval);        // to free space, NO need to free(el->v.fsval)! That will do fs_free() because of FS_FLAG_MOVED
        break;
        default:
            // nothing here
        break;
    }
    free(el);
}
// return count of deleted elems
static int                  free_elemlist(hset_elem *el, value64_type typ){
    hset_elem       *next = 0;
    int              cnt = 0;
    while (el){
        next = el->next;
        free_elem(el, typ);
        el = next;
        cnt++;
    }
    return cnt;
}

static bool                 validate_elemlist(const hset_elem *el, value64_type typ, unsigned pos, unsigned sz){
    invraisecode(ERR_NULLABLE_PTR, el != NULL, "Null pointer");
    const hset_elem *prevel = 0;
    int              iternum = 0;
    while (el){
        unsigned hash = get_lhash(sz, el->v, typ);
        if (hash != pos)
            return logsimpleerr(false, "iter %d: %u, but must be %u", iternum, hash, pos);
        if (prevel)
            if (value64_compare(el->v, prevel->v, typ) < 0)
                return logsimpleret(false, "iter %d: ordering violation", iternum);  // TODO: valprev %s valnext %s must be here via fs
        el = el->next;
    }
    return logsimpleerr(true, "%u Ok", pos);
}

static value64           hset_createarrval(void *arr, int idx, value64_type typ) {
    invraisecode(ERR_NULLABLE_PTR, arr != NULL, "Null pointer");
    size_t elem_size;
    if (typ != VALUE64_FS)
        elem_size = value64_info_get(typ)->size; //hset_elem_sizes[typ];
    else
        elem_size = sizeof(fs); // befause for fs it's fs[], so size of element is sizeof(fs), but not sizeof(fs *)

    if (elem_size == 0)
        userraiseint(ERR_UNSUPPORTED_TYPE, "type %d", typ);

    char *base = (char *) arr;
    // caculate a real offset
    return value64_pmove(base + idx * elem_size, typ);     // hset_createval(base + idx * elem_size, typ);
}


// find every value in the list in se
static bool                 find_elems(const hset_elem *restrict el, const hset *restrict se){
    while (el){
        if (!hset_get(se, el->v) )
            return false;
        el = el->next;
    }
    return true;
}

// ------------------------------------- API -----------------------------------------------

// ---------------------------------- API Constructs/Destrucor  ----------------------------

// value64 constructor ANY type, TODO: will be moved to value64
/*value64                  hset_createval(const void *p, value64_type typ){
    value64 tmp = LITERAL64_ZERO;  // init
    switch (typ){
        case VALUE64_INT:
            tmp.ival = *(const int *) p;
        break;
        case VALUE64_LNG:
            tmp.lval = *(const long *) p;
        break;
        case VALUE64_DBL:
            tmp.dval = *(const double *) p;
        break;
        case VALUE64_PTR:
            tmp.pval = *(void * const *) p;
        break;
        case VALUE64_FS:
            tmp.fsval = *(fs * const *)p; //hset_create_fs(*(const fs * const *) p);         // FS_FLAG_MOVED is set!
        break;
        default:
            userraiseint(ERR_UNSUPPORTED_TYPE, "type %d isn't suppoted", typ);
        break;
    }
    return tmp;
}*/
// hset basic constructor
hset                        hset_init(int sz, value64_type typ){
    logenter("init sz %d - %s", sz, value64_typename(typ) );

    if (int_notin(typ, VALUE64_INT, VALUE64_LNG, VALUE64_DBL, VALUE64_FS, VALUE64_PTR) )
        userraiseint(ERR_UNSUPPORTED_TYPE, "%d", typ);

    if (sz <= 0)
        sz = 1;
    unsigned    newsz = next_prime(sz);
    hset res = HSET(newsz, typ);
    if ( (res.table = malloc(newsz * sizeof(hset_elem *) ) ) == 0)    // raise here - nothing to do
        userraiseint(ERR_UNABLE_ALLOCATE, "Unable to alloc newsz %u elems", newsz);
    clean_ptr( (void *) res.table,  newsz);

    return logret(res, "Created with %d", res.sz);
}
// construct via new hash table size
hset             hset_init_resize(hset *se, int newsz){
    invraisecode(ERR_NULLABLE_PTR, se != 0, "Null pointer");

    if (next_prime(newsz) == (unsigned) se->sz)
        return logsimpleret(*se, "No change");
    hset    res = hset_init(newsz, getype(se) );
    // hset_foreach(se)
    for (int i = 0; i < se->sz; i++){
        hset_elem *el = se->table[i];
        while (el){
            hset_elem *next = el->next;
            hset_elem_move(&res, el); // el->next will be modified!
            el = next;  // go to next elem of ordered list
        }
    }
    // clean the table manually
    free(se->table);
    //hset_free(se);  // NO NEED cleanup
    *se = res;  // and fill with newly created
    return logsimpleret(res, "Resized to %d", res.sz);
}
// normalization
hset                        hset_normalize(hset *se){
    invraise(se != 0, "Null pointer");

    double load_factor = (double)se->count / se->sz;
    // Изменяем размер только при выходе из желаемого диапазона
    if (load_factor > 0.75 || load_factor < 0.25) {
        int     newsz = (int) (se->count * HSET_NORMALIZE_FACTOR);   // или любой другой коэффициент
        if (newsz < HSET_MIN_SIZE)
            newsz = HSET_MIN_SIZE;                   // минимальный размер
        return hset_init_resize(se, newsz);
    }
    return logsimpleret(*se, "Normilized to %d / %d", se->sz, se->count);
}
// cloning
hset                        hset_clone(const hset *se){
    invraise(se != 0, "Null pointer");

    int     newsz = se->sz;
    hset    res = hset_init(newsz - 1, getype(se) );

    if ( (res.table = malloc(newsz * sizeof(hset_elem *) ) ) == 0)    // raise here - nothing to do
        userraiseint(ERR_UNABLE_ALLOCATE, "Unable to alloc newsz %u elems", newsz);

    for (int i = 0; i < res.sz; i++){
        res.table[i] = clone_elemlist(se->table[i], getype(se) );
    }
    res.count = se->count;

    return logsimpleret(res, "Cloned");
}
// created with new type only (INT, LONG, DOUBLE, FS) as allowed
hset                        hset_cloneas(const hset *se, value64_type typ){
    invraise(se != 0, "Null pointer");

    if (int_notin(typ, VALUE64_INT, VALUE64_LNG, VALUE64_DBL, VALUE64_FS) && int_notin(getype(se), VALUE64_INT, VALUE64_LNG, VALUE64_DBL, VALUE64_FS))
        userraiseint(ERR_UNSUPPORTED_TYPE, "From %d:%s - to %d:%s",
            getype(se), value64_typename(getype(se) ), typ, value64_typename(typ) );

    hset    res = hset_init(se->sz - 1, typ);
    for (int i = 0; i < se->sz; i++){
        const hset_elem *el = se->table[i];     // probably better to create separate function
        while (el){
            if (!hset_set(&res, value64_convert(el->v, getype(se), typ) ) )
                userraiseint(ERR_UNABLE_ALLOCATE, "Unable to create element");
            el = el->next;
        }
    }
    return logsimpleret(res, "Cloned as %d:%s", typ, value64_typename(typ) );
}

hset                        hset_from_anyarr(const void *arr, int sz, value64_type typ){
    invraisecode(ERR_NULLABLE_PTR, arr != 0 && sz > 0 && sz < INT_MAX / 4, 
            "Incorrent input %p - %d", arr, sz);
    if (int_notin(typ, VALUE64_INT, VALUE64_LNG, VALUE64_DBL, VALUE64_PTR, VALUE64_FS) )
        userraiseint(ERR_UNSUPPORTED_TYPE, "%d", typ);

    if (sz <= 0)
        sz = 1;     // to avoid 0 initializing
    hset        res = hset_init(sz * HSET_ARRAY_CREATE_MULTIPLIER, typ);
    int         cnt = hset_loadanyarr(&res, (void *) arr, sz, typ);     // (void *) because of fs
    return logsimpleret(res, "Created hest with sz %u, loaded %d", res.sz, cnt);
}

void                        hset_free(hset *se){
    invraise(se != 0, "Null pointer");

    hset_clean(se);
    free(se->table);
    se->sz = se->flags = 0;
    if (hset_heap_alloc(se) ){
        free(se);
        logsimple("Heap freed");
    }
    logsimple("freed");
}
// return new hset se1 - se2
hset             hset_init_minus(const hset *restrict se1, const hset *restrict se2){
    invraisecode(ERR_NULLABLE_PTR, se1 != 0 && se2 != 0, "Null pointers %p %p", se1, se2);

    if (getype(se1) != getype(se2) )
         userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", value64_typename(getype(se1)), value64_typename(getype(se2) ) );

    // simple implementation
    hset res = hset_init(se1->sz - 1, se1->flags);
    for (int i = 0; i < se1->sz; i++){
        const hset_elem *el = se1->table[i];
        while (el){
            if (!hset_get(se2, el->v) )
                hset_set(&res, el->v);
            el = el->next;
        }
    }
    return logsimpleret(res, "Created minus - total %d", se1->count);
}
// just intersect with construct
hset                        hset_init_intersect(const hset *restrict se1, const hset *restrict se2){
    invraisecode(ERR_NULLABLE_PTR, se1 != 0 && se2 != 0, "Null pointers %p %p", se1, se2);

    if (getype(se1) != getype(se2) )
          userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", value64_typename(getype(se1)), value64_typename(getype(se2) ) );

    // simple implementation
    hset res = hset_init(se1->sz - 1, se1->flags);
    if (se1->count > 0 && se2->count > 0)
        for (int i = 0; i < se1->sz; i++){
            const hset_elem *el = se1->table[i];
            while (el){
                if (hset_get(se2, el->v) )  // omg
                    hset_set(&res, el->v);
                el = el->next;
            }
        }
    return logsimpleret(res, "Created intersect - total %d", res.count);
}
// simm diff with construct
hset                        hset_init_symmdiff(const hset *restrict se1, const hset *restrict se2){
    invraisecode(ERR_NULLABLE_PTR, se1 != 0 && se2 != 0, "Null pointers %p %p", se1, se2);
    if (getype(se1) != getype(se2) )
         userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", value64_typename(getype(se1)), value64_typename(getype(se2) ) );
    hset res = hset_init(MAX(se1->sz, se2->sz) - 1, se1->flags);

    for (int i = 0; i < se1->sz; i++){
        const hset_elem *el = se1->table[i];
        while (el){
            if (!hset_get(se2, el->v))
                hset_set(&res, el->v);
            el = el->next;
        }
    }
    for (int i = 0; i < se2->sz; i++){
        const hset_elem *el = se2->table[i];
        while (el){
            if (!hset_get(se1, el->v))
                hset_set(&res, el->v);
            el = el->next;
        }
    }

    return logsimpleret(res, "Created symm diff - total %d", res.count);
}
// union with construct
hset                        hset_init_union(const hset *restrict se1, const hset *restrict se2){
    invraisecode(ERR_NULLABLE_PTR, se1 != 0 && se2 != 0, "Null pointers");
    if (getype(se1) != getype(se2) )
         userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", value64_typename(getype(se1)), value64_typename(getype(se2) ) );
    hset res = hset_init(MAX(se1->sz, se2->sz) - 1, se1->flags);

    for (int i = 0; i < se1->sz; i++){
        const hset_elem *el = se1->table[i];
        while (el){
            hset_set(&res, el->v);
            el = el->next;
        }
    }
    for (int i = 0; i < se2->sz; i++){
        const hset_elem *el = se2->table[i];
        while (el){
            hset_set(&res, el->v);
            el = el->next;
        }
    }
    return logsimpleret(res, "Created union - total %d", res.count);
}

// ---------------------------------------------------------------------------
bool                        hset_validate(FILE *out, const hset *restrict se){
    logenter("...");
    if (!se){    // TODO: think about string creation via faststring here!!
        if (out)
            fprintf(out, "null pointer");
        return logerr(false, "null pointer");
    }
    if (se->sz <= 1){
        if (out)
            fprintf(out, "Incorrect sz (%d)", se->sz);
        return logerr(false, "Incorrect sz (%d)", se->sz);
    }
    // check count agaist hset_cnt()
    int cnt = hset_calc_cnt(se);
    if (se->count != cnt){
        if (out)
            fprintf(out, "Cnt %d not matched to calculated cnt %d", se->count, cnt);
        return logerr(false, "Cnt %d not matched to calculated cnt %d", se->count, cnt);
    }
    if (int_notin(getype(se), VALUE64_INT, VALUE64_LNG, VALUE64_DBL, VALUE64_FS, VALUE64_PTR) ){
        if (out)
            fprintf(out, "Incorrect type %d", getype(se) );
        return logerr(false, "Incorrect type %d", getype(se) );
    }
    if (!se->table){
        if (out)
            fprintf(out, "null hashtable");
        return logerr(false, "null hashtable");
    }
    // cross validation value <=> hashkey
    for (int i = 0; i < se->sz; i++)
        if (se->table[i] )
            if (!validate_elemlist(se->table[i], getype(se), i, se->sz) ){
                if (out)
                    fprintf(out, "Hash Validation failed on %d", i);
                return logerr(false, "Hash Validation failed on %d", i);
            }

    return logret(true, "Validated");
}

// ---------------------------------- General functions ------------------------------------

bool                        hset_set(hset *se, value64 val){
    invraisecode(ERR_NULLABLE_PTR, se != 0, "Null pointer");

    return create_or_move_elem(se, 0, val);
}
// move el from one hset to target se
bool                        hset_elem_move(hset * restrict se, hset_elem *restrict el){
    invraise(se != 0 && el != 0, "Null pointers");
    // No checking type here
    return create_or_move_elem(se, el, LITERAL64_ZERO);
}

bool                        hset_get(const hset *se, value64 val){
    invraisecode(ERR_NULLABLE_PTR, se != 0, "Null pointer");

    hset_elem   *equal = 0;
    getprevelem(se, val, 0, 0, &equal);
    if (equal){
        //hsetval_log(val, getype(se) );
        return true;
    } else {
        //hsetval_log(val, getype(se) );
        return false;
    }
}
// try to delete elemenet, true if deleted, false if not found
bool                        hset_del(hset *se, value64 val){
    invraisecode(ERR_NULLABLE_PTR, se != 0, "Null pointer");

    unsigned            hash;
    hset_elem          *el = 0;
    hset_elem          *prevel = getprevelem(se, val, &hash, 0, &el);
    if (!el)    // not found
        return logsimpleerr(false, "Not found");
    // umount el
    if (!prevel)
         se->table[hash] = el->next;
    else
        prevel->next = el->next;
    free_elem(el, getype(se) );
    se->count--;
    //hsetval_log(val, getype(se) );
    return logsimpleret(true, "Deleted");
}

void                        hset_clean(hset *se){
    invraise(se != 0, "Null pointer");

    for (int i = 0; i < se->sz; i++)
        if (se->table[i] ){
            se->count -= free_elemlist(se->table[i], getype(se) );
            se->table[i] = 0;   // clean that chain
        }
}
// check the equality as SET!
bool                        hset_eq(const hset *restrict se1, const hset *restrict se2){
    invraise(se1 != 0 && se2 != 0, "Null pointers");

    if (getype(se1) != getype(se2) )
        return userraise(false, ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", value64_typename(getype(se1)), value64_typename(getype(se2) ) );

    int     cnt1 = hset_cnt(se1);
    int     cnt2 = hset_cnt(se2);
    if (cnt1 != cnt2 )
        return logsimpleret(false, "Count's not equal %d vs %d", cnt1, cnt2);
    // chech the elements
    bool        res = true;
    for (int i = 0; i < se1->sz; i++)
        if (!find_elems(se1->table[i], se2) ){
            res = false;
            break;
        }

    return logsimpleret(res, "Equal %s", bool_str(res) );
}

// check NOT equality as SET
bool                        hset_noteq(const hset *restrict se1, const hset *restrict se2){
    invraisecode(ERR_NULLABLE_PTR, se1 != 0 && se2 != 0, "Null pointers %p %p", se1, se2);

    if (getype(se1) != getype(se2) )
        return userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", value64_typename(getype(se1)), value64_typename(getype(se2)));

    int     cnt1 = hset_cnt(se1);
    int     cnt2 = hset_cnt(se2);
    if (cnt1 != cnt2 )
        return logsimpleret(true, "Not equal counts %d vs %d", cnt1, cnt2);
    bool    res = false;
    for (int i = 0; i < se1->sz; i++)
        if (!find_elems(se1->table[i], se2) ){
            res = true;
            break;  // found at least 1 elem which not match to se2
        }
    return logsimpleret(res,  "Equal %s", bool_str(!res) );
}
// for array mass loading
int                         hset_loadanyarr(hset *restrict se, void *arr, int sz, value64_type typ){ 
    invraisecode(ERR_NULLABLE_PTR, arr != 0 && sz > 0 && sz < INT_MAX / 4, "Incorrent input %p - %d", arr, sz);
    if (int_notin(typ, VALUE64_INT, VALUE64_LNG, VALUE64_DBL, VALUE64_PTR, VALUE64_FS) )
        userraiseint(ERR_UNSUPPORTED_TYPE, "%d - %s", typ, value64_typename(typ) );

    int         cnt = 0;
    while (cnt < sz){
        // created a simple element from arr
        value64 val = hset_createarrval(arr, cnt, typ);
        if (!hset_set(se, val) )
            if (typ == VALUE64_FS) {
                // currently avoiding dupl only for fs
                logsimple("DUPLICATE freeing %p [%s]", val.fsval, val.fsval ? val.fsval->v : "");
                value64freefs(val);
            }
        cnt++;
    }
    return logsimpleret(cnt, "Loaded total %d", cnt);
}
// fs loading, heap allocated => MOVE semantic!
int                         hset_loadfs_str(hset *restrict se, const char *strings[]){
    invraisecode(ERR_NULLABLE_PTR, strings != NULL && se != NULL, 
            "Null pointers %p %p", se, strings);
    int     cnt = 0;
    while (*strings) {
        // convent c-str into fs * (heap allocated body)
        value64  val = value64_createfs_asstr(*strings);
        if (!hset_set(se, val) )
            value64_freefs(&val);   // TODO: that need's to be reworked to avoid useless alloc/free when dublicate
        strings++;
        cnt++;
    }
    return logsimpleret(cnt, "Loaded(moved) c-str total %d", cnt);
}
/*
// only static literals!
int                         hset_loadfs_literal(hset *restrict se, const char *lits[]){
    int     cnt = 0;
    while (*lits) {
        fs l = FSLITERAL( (char *) *lits); // no allocatopn here
        value64 val = ???? somehow create fs_create() and link lits with static alloc
        hset_set(se, val);
        lits++;
        cnt++;
    }
    return logsimpleret(cnt, "Loaded literal total %d", cnt);
} */

// check if all of se2 in se1 strictly or not
bool                        hset_subset_check(const hset *restrict se1, const hset *restrict se2, bool strict){
    invraisecode(ERR_NULLABLE_PTR, se1 != 0 && se2 != 0, "Null pointers %p %p", se1, se2);

    if (getype(se1) != getype(se2) )
        userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", value64_typename(getype(se1) ), value64_typename(getype(se2) ) );

    if ( (!strict && hset_cnt(se1) > hset_cnt(se2) ) ||
        (strict && hset_cnt(se1) >= hset_cnt(se2) ) )
        return logsimpleret(false, "Count of se1 %d more (%s) that count of se2 %d", hset_cnt(se1), bool_str(strict), hset_cnt(se2) );
    // check that EVERY element in se1 exists in se2
    for (int i = 0; i < se1->sz; i++){
        const hset_elem *el = se1->table[i];
        while (el){
            if (!hset_get(se2, el->v) )
                return logsimpleret(false, "Not matched as in");
            el = el->next;
        }
    }
    return logsimpleret(true, "Matched %s as in", strict ? "strict": "");
}
// if not exists se2 in se1
bool                        hset_notexists(const hset *restrict se1, const hset *restrict se2){
    invraisecode(ERR_NULLABLE_PTR, se1 != 0 && se2 != 0, "Null pointers %p %p", se1, se2);
    // refsctor tha to common
    if (getype(se1) != getype(se2) )
        userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", value64_typename(getype(se1) ), value64_typename(getype(se2) ) );
    HSET_FOREACH(se2, val){
        if (hset_get(se1, val) ){
            value64_fprint(logfile, val, getype(se2));
            return logsimpleret(false, "Found element in se1");
        }
    }
    return logsimpleret(true, "Not found - OK");
}
// if exists any of se2 in se1
bool                        hset_any(const hset *restrict se1, const hset *restrict se2){
    invraisecode(ERR_NULLABLE_PTR, se1 != 0 && se2 != 0, "Null pointers %p %p", se1, se2);
    // refsctor tha to common
    if (getype(se1) != getype(se2) )
        userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", value64_typename(getype(se1) ), value64_typename(getype(se2) ) );
    HSET_FOREACH(se2, val){
        if (hset_get(se1, val) ){
            value64_fprint(logfile, val, getype(se2));
            return logsimpleret(true, "Element of se2 Found in se1");
        }
    }
    return logsimpleret(false, "All not found - OK");
}
// se1 -= se2 as SET, returns count of deleted element
hset                       *hset_minus(hset *restrict se1, const hset *restrict se2){
    invraisecode(ERR_NULLABLE_PTR, se1 != 0 && se2 != 0, "Null pointers %p %p", se1, se2);
    // TODO: rework checkers!!! at least move that into check_it()
    if (getype(se1) != getype(se2) )
        userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", value64_typename(getype(se1) ), value64_typename(getype(se2) ) );

    int     cnt = 0;
    if ( !(hset_cnt(se1) == 0 || hset_cnt(se2) == 0) ){
        // foreach elements in se2 try to delete it from se1
        for (int i = 0; i < se2->sz; i++){
            const hset_elem *el = se2->table[i];
            while (el){
                cnt += (int) hset_del(se1, el->v);    // +1 if reallt deleted
                el = el->next;
            }
        }
    }
    return logsimpleret(se1, "Removed %d elements", cnt);
}

// se1 insersect= se2 as SET
hset                       *hset_intersect(hset *restrict se1, const hset *restrict se2){
    invraisecode(ERR_NULLABLE_PTR, se1 != 0 && se2 != 0, "Null pointers %p %p", se1, se2);
    // TODO: rework checkers!!! at least move that into check_it()
    if (getype(se1) != getype(se2) )
        userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", value64_typename(getype(se1) ), value64_typename(getype(se2) ) );

    // foreach elements in se2 try to delete it from se1
    int     delcnt = 0;
    for (int i = 0; i < se1->sz; i++){
        const hset_elem *el = se1->table[i];
        while (el){
            hset_elem *next = el->next;
            if (!hset_get(se2, el->v) )
                delcnt += (int) hset_del(se1, el->v);    // +1 if reallt deleted
            el = next;
        }
    }
    return logsimpleret(se1, "%d elements remains, deleted %d", se1->count, delcnt);
}

// se1 symmdiff= se2 as SET
hset                       *hset_symmdiff(hset *restrict se1, const hset *restrict se2){
    invraisecode(ERR_NULLABLE_PTR, se1 != 0 && se2 != 0, "Null pointers %p %p", se1, se2);
    // TODO: rework checkers!!! at least move that into check_it()
    if (getype(se1) != getype(se2) )
        userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", value64_typename(getype(se1) ), value64_typename(getype(se2) ) );

    // done via constructor
    hset tmp = hset_init_symmdiff(se1, se2);
    hset_move(se1, &tmp);       // no need to free tmp!!!!
    return logsimpleret(se1, "Done, new cnt = %d", se1->count);
}
// union= as SET
hset            *hset_union(hset *restrict se1, const hset *restrict se2){ 
    invraisecode(ERR_NULLABLE_PTR, se1 != 0 && se2 != 0, "Null pointers %p %p", se1, se2);
    // TODO: rework checkers!!! at least move that into check_it()
    if (getype(se1) != getype(se2) )
        userraiseint(ERR_TYPES_MISMATCH, "Incorrect type %s vs %s", value64_typename(getype(se1) ), value64_typename(getype(se2) ) );

    int         addcnt = 0;
    if (se2->count > 0){
        for (int i = 0; i < se2->sz; i++){
            const hset_elem *el = se2->table[i];
            while (el){
                addcnt += hset_set(se1, el->v);
                el = el->next;
            }
        }
    }
    return  logsimpleret(se1, "%d elements remains, added from se2 %d", se1->count, addcnt);
}

// ------------------------------------- (API) printers ------------------------------------

int                         hset_techfprint(FILE *restrict out, const hset *se, int sz, const char *restrict name){
    invraisecode(ERR_NULLABLE_PTR, se != 0, "Null pointer %p", se);

    int     cnt = 0;
    if (out){
        sz = sz ? MIN(sz, se->sz) : se->sz;
        logauto(sz);
        cnt += fprintf(out, "HSET %s(sz %d, flags %d, typez %s)[\n", name ? name : "", se->sz, se->flags, value64_typename(getype(se) ) );
        for (int i = 0; i < sz; i++)
            if (se->table[i]){
                cnt += fprintf(out, "%4d: ", i);
                cnt += sortedlist_fprint(out, se->table[i], getype(se) );
                cnt += fprintf(out, "\n");
            }
        cnt += fprintf(out, "]\n");
    }
    return cnt;
}

// --------------------------------- SERIALIZATION -----------------------------------------

int                         hset_fsave(FILE  *restrict out, const hset *se) {
    invraisecode(se != NULL, ERR_NULLABLE_PTR,
                "Null pointer");

    value64_type   typ = getype(se);

    invraisecode ( int_in(typ, VALUE64_INT, VALUE64_LNG, VALUE64_DBL, VALUE64_PTR),
                ERR_UNSUPPORTED_TYPE,
                "%d - %s", typ, value64_typename(typ) );
    int         cnt = 0;
    if (out)
        fprintf(out, "HSET: %s : %d\n", value64_typename(typ), se->count);

    for (int i = 0; i < se->sz; i++) {
        const hset_elem *el = se->table[i];
        while (el) {
            value64_fprint(out, el->v, typ);
            el = el->next;
            cnt++;
            fputc('\n', out);
        }
    }
    if (out)
        fprintf(out, "HSET: DONE\n");
    return logsimpleret(cnt, "Saved %d", cnt);
}

int                         hset_fload(FILE *restrict in, hset *restrict se) {
    invraisecode(in != NULL && se != 0, ERR_NULLABLE_PTR, "Null pointer");

#define HSET_LOAD_BUF_SIZE 200
#define HSET_LOAD_BUF_FMT "199"   /* HSET_LOAD_BUF_SIZE - 1 */

    char        buf[HSET_LOAD_BUF_SIZE];
    int         cnt = 0;
    hset        res;
    hset       *target = se;
    bool        must_free_on_error = false;

    //  TODO: rework!!! via fs_fscanf()
    if (fscanf(in, " HSET: %" HSET_LOAD_BUF_FMT "s : %d ", buf, &cnt) != 2)
        return userraise(-1, ERR_WRONG_INPUT_FORMAT, "Invalid header");

    logsimple("[%s]", buf);
    value64_type   file_type = value64_gettype(buf);
    logauto(file_type);

    if (!hset_isnoninit(se) && file_type != getype(se))
        return userraise(-1, ERR_WRONG_INPUT_FORMAT, "Type mismatch: set %s, file %s",
                      value64_typename(getype(se)), buf);

    if (hset_isnoninit(se) ){
        res = hset_init(cnt, file_type);
        must_free_on_error = true;
        target = &res;
    }

    int     addcnt = 0;
    fs      buf1 = FS();                // TODO: rework of hset_fload is requied to use onle 1 fs buf!!!
    for (int i = 0; i < cnt; i++) {
        value64 val = LITERAL64_ZERO;
        if (!value64_freadval(in, file_type, &val, &buf1)) {
            if (must_free_on_error)
                hset_free(&res);
            return userraise(-1, ERR_WRONG_INPUT_FORMAT, "Failed to read element %d", i);
        }
        addcnt += hset_set(target, val);    // addcnt++ only if real adding
    }

    // Проверяем завершающую строку "HSET: DONE"
    if (fscanf(in, " HSET: %" HSET_LOAD_BUF_FMT "s", buf) != 1 || strcmp(buf, "DONE") != 0){
        if (must_free_on_error)
            hset_free(&res);
        fsfree(buf1);
        return userraise(-1, ERR_WRONG_INPUT_FORMAT, "Missing or invalid 'HSET: DONE'");
    }
    if (hset_isnoninit(se) )
        *se = res;
    fsfree(buf1);

    return logsimpleret(addcnt, "Loaded %d/%d elements into set", addcnt, se->count);
}

int                         hset_save(const char *restrict fname, const hset *se) {
    invraisecode(se != NULL && fname != NULL, ERR_NULLABLE_PTR, "Null pointer");

    FILE *f = fopen(fname, "w");
    if (!f)
        return logsimpleret(-1, "Unable to open file %s", fname);

    int         cnt = hset_fsave(f, se);

    fclose(f);
    return logsimpleret(cnt, "Saved %d into %s", cnt, fname);
}

int                         hset_load(const char *restrict fname, hset *restrict se) {
    invraisecode(fname != NULL, ERR_NULLABLE_PTR, "Null pointer");

    FILE *f = fopen(fname, "r");
    if (!f)
        return userraise(-1, ERR_UNABLE_OPEN_FILE_READ, "Cannot open '%s'", fname);

    int    res = hset_fload(f, se);
    fclose(f);
    return logsimpleret(res, "%d loaded from '%s'", res, fname);
}

// --------------------------------------- ITERATORS ---------------------------------------

void                        hset_const_foreach(const hset *se, hset_const_proc_t proc) {
    for (int i = 0; i < se->sz; i++) {
        const hset_elem *el = se->table[i];
        while (el) {
            proc(el->v);   // передаём копию
            el = el->next;
        }
    }
}
/*      NOT USED FOR NOW
void                        hset_foreach(hset *se, hset_proc_t proc) {
    for (int i = 0; i < se->sz; i++) {
        hset_elem *el = se->table[i];
        while (el) {
            proc(&el->v);
            el = el->next;
        }
    }
}
void                        hset_modify_foreach(hset *se, hset_modify_proc_t proc) {
    for (int i = 0; i < se->sz; i++) {
        hset_elem *el = se->table[i];
        while (el) {
            hset_elem *next = el->next;
            proc(se, el);   // можно вызывать hset_del внутри
            el = next;
        }
    }
} */
// ----------------------------------------- REDUCE -----------------------------------------

hset_accum                  hset_initreduce(const hset *se, hset_accum init, hset_reduce_func func) {
    hset_accum acc = init;
    for (int i = 0; i < se->sz; i++) {
        const hset_elem *el = se->table[i];
        while (el) {
            func(&acc, el->v);
            el = el->next;
        }
    }
    return acc;
}
// ------------------------------------- REDUCE IMPL -----------------------------------------
void                        hset_sum_int(hset_accum *acc, value64 v) {
    acc->value.ival += v.ival;
    acc->count++;
}

void                        hset_count_int(hset_accum *acc, value64 v) {
    (void) v;
    acc->count++;
}

void                        hset_max_int(hset_accum *acc, value64 v) {
    if (acc->count == 0 || v.ival > acc->value.ival) {
        acc->value.ival = v.ival;
        acc->count = 1;   // помечаем, что значение уже есть
    }
}

void                        hset_min_int(hset_accum *acc, value64 v) {
    if (acc->count == 0 || v.ival < acc->value.ival) {
        acc->value.ival = v.ival;
        acc->count = 1;
    }
}

/*void                        hset_avg_int(hset_accum *acc, value64 v) {
    acc->value.ival += v.ival;
    acc->count++;
}*/
// double
void                        hset_sum_dbl(hset_accum *acc, value64 v) {
    if (isfinite(v.dval)) {
        acc->value.dval += v.dval;
        acc->count++;
    }
}

void                        hset_count_dbl(hset_accum *acc, value64 v) {
    if (isfinite(v.dval))
        acc->count++;
}

void                        hset_max_dbl(hset_accum *acc, value64 v) {
    if (!isfinite(v.dval))
        return;
    if (acc->count == 0 || v.dval > acc->value.dval) {
        acc->value.dval = v.dval;
        acc->count = 1;   // помечаем, что значение уже есть
    }
}

void                        hset_min_dbl(hset_accum *acc, value64 v) {
    if (!isfinite(v.dval))
        return;
    if (acc->count == 0 || v.dval < acc->value.dval) {
        acc->value.dval = v.dval;
        acc->count = 1;
    }
}

/*void                        hset_avg_dbl(hset_accum *acc, value64 v) {
    if (isfinite(v.dval)) {
        acc->value.dval += v.dval;
        acc->count++;
    }
}*/

// ---------------------------------------- Testing ------------------------------------------
#ifdef HSETTESTING

#include "test.h"
#include "array.h"
#include <time.h>

//types for testing

// ------------------------- TEST 1 ---------------------------------

static TestStatus
tf1(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: init + free", ++subnum);
    {
        hset se1 = hset_init(100, VALUE64_INT);
        hset_tech_fprintall(logfile, se1);
        test_validatefree(
            hset_validate(stdout, &se1), hset_free(&se1), "Validation failed"
        );
        hset_free(&se1);
    }
    test_sub("subtest %d: init + free", ++subnum);
    {
        hset sefs1 = hset_init(100, VALUE64_FS);
        hset_tech_fprintall(logfile, sefs1);
        test_validatefree(
            hset_validate(stdout, &sefs1), hset_free(&sefs1), "Validation failed"
        );
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------- TEST 2 ---------------------------------

static TestStatus
tf2(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: VALUE64_INT init + get + free", ++subnum);
    {
        hset    se1 = hset_init(100, VALUE64_INT);
        int     num = 77;
        // mustb return false
        test_validatefree(
            hset_set(&se1, LITERAL64_INT(num) ),
            hset_free(&se1), "Must be true"
        );
        hset_tech_fprintall(logfile, se1);
        test_validatefree(
            hset_set(&se1, LITERAL64_INT(num) ) == false,
            hset_free(&se1), "Must be false because elem %d aleady in the set", num
        );
        hset_tech_fprintall(logfile, se1);
        test_validatefree(
            hset_validate(stdout, &se1), hset_free(&se1), "Validation failed"
        );
        test_validatefree(
            hset_get(&se1, LITERAL64_INT(num) ), hset_free(&se1), "Must be true,  because already added %d", num
        );
        test_validatefree(
            hset_get(&se1, LITERAL64_INT(num + 1) ) == false, hset_free(&se1), "Must be false %d", num + 1
        );
        test_validatefree(
            hset_del(&se1, LITERAL64_INT(num) ), hset_free(&se1), "Must be true, because element %d exists", num
        );
        hset_tech_fprintall(logfile, se1);
        test_validatefree(
            hset_del(&se1, LITERAL64_INT(num) ) == false, hset_free(&se1), "Must be false, because element %d already deleted", num
        );
        test_validatefree(
            hset_validate(stdout, &se1), hset_free(&se1), "Validation failed"
        );
        hset_free(&se1);
    }
    test_sub("subtest %d: VALUE64_INT multiple add/del", ++subnum);
    {
        int     cnt = 100, mul = 3;
        hset    se1 = hset_init(cnt, VALUE64_INT);

        for (int i = 0; i < cnt * mul; i++)
            test_validatefree(
                hset_set(&se1, LITERAL64_INT(i) ), hset_free(&se1), "Already exists %d", i
            );
        hset_tech_fprintall(logfile, se1);

        for (int i = 0; i < cnt * mul; i++)
            test_validatefree(
                hset_get(&se1, LITERAL64_INT(i) ), hset_free(&se1), "Unable to get %d", i
            );

        int     delfrom = 40, delto = 50;
        for (int i = delfrom; i < delto; i++)
            test_validatefree(
                hset_del(&se1, LITERAL64_INT(i) ), hset_free(&se1), "Unable to delete %d", i
            );

        for (int i = 0; i < cnt * mul; i++)
            if (i >= delfrom && i < delto){
                test_validatefree(
                    hset_get(&se1, LITERAL64_INT(i) ) == false, hset_free(&se1), "Element %d was deleted, but return found somehow!", i
                );
            } else {
                test_validatefree(
                    hset_get(&se1, LITERAL64_INT(i) ), hset_free(&se1), "Element %d not found", i
                );
            }
        test_validatefree(
            hset_validate(stdout, &se1), hset_free(&se1), "Validation failed"
        );

        hset_free(&se1);
    }
    test_sub("subtest %d: VALUE64_INT multiple reversed add/del", ++subnum);
    {
        int     cnt = 100, mul = 3;
        hset    se1 = hset_init(cnt, VALUE64_INT);

        for (int i = cnt * mul - 1; i >= 0; i--)
            test_validatefree(
                hset_set(&se1, LITERAL64_INT(i) ), hset_free(&se1), "Already exists %d", i
            );

        for (int i = 0; i < cnt * mul; i++)
            test_validatefree(
                hset_get(&se1, LITERAL64_INT(i) ), hset_free(&se1), "Unable to get %d", i
            );
        int     delfrom = 40, delto = 50;
        for (int i = delfrom; i < delto ; i++)
            test_validatefree(
                hset_del(&se1, LITERAL64_INT(i) ), hset_free(&se1), "Unable to delete %d", i
            );

        for (int i = 0; i < cnt * mul; i++)
            if (i >= delfrom && i < delto){
                test_validatefree(
                    hset_get(&se1, LITERAL64_INT(i) ) == false, hset_free(&se1), "Element %d was deleted, but return found somehow!", i
                );
            } else {
                test_validatefree(
                    hset_get(&se1, LITERAL64_INT(i) ), hset_free(&se1), "Element %d not found", i
                );
            }

        hset_free(&se1);
    }
    test_sub("subtest %d: random int add/del", ++subnum);
    {
        int     cnt = 100, mul = 3;
        hset    se1 = hset_init(cnt, VALUE64_INT);

        for (int i = cnt * mul - 1; i >= 0; i--)
            test_validatefree(
                hset_set(&se1, LITERAL64_INT(i) ), hset_free(&se1), "Already exists %d", i
            );
        // random del
        srand(time(NULL));
        for (int i = 0; i < cnt * mul; i += rndint(5) + 1 )
            //if (i < cnt * mul)
                test_validatefree(
                    hset_del(&se1, LITERAL64_INT(i) ), hset_free(&se1), "Unable to delete [%d]", i
                );

        test_validatefree(
            hset_validate(stdout, &se1), hset_free(&se1), "Validation failed"
        );
        hset_free(&se1);
    }

    test_sub("subtest %d: FS init + add + get + free", ++subnum);
    {
        hset se = hset_init(10, VALUE64_FS);

        fs s1 = fscopy("hello");   // локальная копия строки
        fs s2 = fscopy("world");
        fs s3 = fscopy("foo");

        // Вставляем (hset_createval сделает свою копию)
        test_validatefree(
            hset_set(&se, LITERAL64_FS(s1)) == true,
            hset_free(&se),
            "Failed to insert 'hello'"
        );
        test_validatefree(
            hset_set(&se, LITERAL64_FS(s2)) == true,
            hset_free(&se),
            "Failed to insert 'world'"
        );
        test_validatefree(
            hset_set(&se, LITERAL64_FS(s3)) == true,
            hset_free(&se),
            "Failed to insert 'foo'"
        );

        // Проверяем наличие (s1, s2, s3 всё ещё валидны)
        test_validatefree(
            hset_get(&se, LITERAL64_FS(s1)) == true,
            hset_free(&se),
            "Missing 'hello'"
        );
        test_validatefree(
            hset_get(&se, LITERAL64_FS(s2)) == true,
            hset_free(&se),
            "Missing 'world'"
        );
        test_validatefree(
            hset_get(&se, LITERAL64_FS(s3)) == true,
            hset_free(&se),
            "Missing 'foo'"
        );

        test_validatefree(
            hset_cnt(&se) == 3,
            hset_free(&se),
            "Count should be 3, got %d", hset_cnt(&se)
        );

        // Очищаем исходные строки (они больше не нужны)
        fsfree(s1); fsfree(s2); fsfree(s3);
        hset_free(&se);
    }
    fs_alloc_check(true);
    test_sub("subtest %d: FS empty \"\" checking", ++subnum);
    {
        hset    se = hset_init(10, VALUE64_FS);
        fs      s1 = fsliteral("");
        fs      s2 = fscopy("");
        test_validatefree(
            hset_set(&se, LITERAL64_FS(s1)) == true,
            hset_free(&se),
            "Failed to insert empty literal"
        );
        hset_clean(&se);
        test_validatefree(
            hset_set(&se, LITERAL64_FS(s2)) == true,
            hset_free(&se),
            "Failed to insert empty fs"
        );
        test_validatefree(
            hset_set(&se, LITERAL64_FS(s1)) == false,
            hset_free(&se),
            "Shiuld be false, because empty line again (literal)"
        );
        fsfree(s2);
        hset_free(&se);
    }
    fs_alloc_check(true);
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------- TEST 3 ---------------------------------

static TestStatus
tf3(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: clone...", ++subnum);
    {
        hset    se1 = hset_init(10, VALUE64_LNG);
        int     cnt = 50;

        for (int i = 0; i < cnt; i++)
            test_validatefree(
                hset_set(&se1, LITERAL64_LNG(i) ), hset_free(&se1), "Already exists %d", i
            );
        hset    se2 = hset_clone(&se1);
        hset_tech_printall(se2);
        // then compare one by one
        bool        r1, r2;
        for (int i = 0; i < cnt; i++){
            r1 = hset_get(&se1, LITERAL64_LNG(i) );
            r2 = hset_get(&se2, LITERAL64_LNG(i) );
            test_validatefree(
                (r1 ^ r2) == false, (hset_free(&se1), hset_free(&se2) ),
                "%d: Must be true all, but origin %s, clone %s", i, bool_str(r1), bool_str(r2)
            );
        }

        hset_free(&se1);

        hset_tech_fprintall(logfile, se2);
        test_validatefree(
            hset_validate(stdout, &se2), hset_free(&se2), "Validation failed"
        );

        hset_free(&se2);
    }
    test_sub("subtest %d: create from int array", ++subnum);
    {
        Array arr = IArray_create(200, ARRAY_RND);

        hset    se1 = hset_from_intarr(arr.iv, arr.len);

        hset_tech_fprintall(logfile, se1);

        for (int i = 0; i < arr.len; i++)
            test_validatefree(
                hset_get(&se1, LITERAL64_INT(arr.iv[i]) ), (Array_free(&arr), hset_free(&se1) ),
                "Element %d isn't found", arr.iv[i]
            );

        test_validatefree(
            hset_validate(stdout, &se1), hset_free(&se1), "Validation failed"
        );

        Array_free(&arr);
        hset_free(&se1);
    }
    test_sub("subtest %d: create from int array, contained 0 value", ++subnum);
    {
        Array arr = IArray_create(10, ARRAY_ZERO);
        hset    se1 = hset_from_intarr(arr.iv, arr.len);

        hset_tech_fprintall(stdout, se1);

        for (int i = 0; i < arr.len; i++)
            test_validatefree(
                hset_get(&se1, LITERAL64_INT(arr.iv[i]) ), (Array_free(&arr), hset_free(&se1) ),
                "Element %d isn't found", arr.iv[i]
            );

        Array_free(&arr);
        hset_free(&se1);
    }
    // VALUE64_FS
    /* ---- FS: clone set ---- */
    test_sub("subtest %d: FS clone", ++subnum);
    {
        hset    se1 = hset_init(10, VALUE64_FS);
        const char *words[] = {"alpha", "beta", "gamma", "delta"};
        for (int i = 0; i < COUNT(words); i++) {
            fs tmp = fscopy(words[i]);
            test_validatefree(
                hset_set(&se1, LITERAL64_FS(tmp)),
                hset_free(&se1),
                "Failed to insert '%s'", words[i]
            );
            fsfree(tmp);
        }

        hset    se2 = hset_clone(&se1);

        // Проверяем, что клон содержит те же элементы
        for (int i = 0; i < COUNT(words); i++) {
            fs tmp = fscopy(words[i]);
            test_validatefree(
                hset_get(&se2, LITERAL64_FS(tmp)),
                (hset_free(&se1), hset_free(&se2)),
                "Clone: missing '%s'", words[i]
            );
            fsfree(tmp);
        }

        test_validatefree(
            hset_cnt(&se2) == COUNT(words),
            (hset_free(&se1), hset_free(&se2)),
            "Clone: count = %d, expected %d", hset_cnt(&se2), COUNT(words)
        );

        hset_free(&se1);
        hset_free(&se2);
    }
    fs_alloc_check(true);
    /* ---- FS: create from array of strings ---- */
    test_sub("subtest %d: FS from array", ++subnum);
    {
        const int cnt = 30;
        fs      strings[cnt];   // локальные fs
        //fs     *parr[cnt];      // массив указателей для hset_loadanyarr

        for (int i = 0; i < cnt; i++) {
            strings[i] = fscopyf("item_%d", i);   // если fscopyf ещё нет, используйте fscopy + snprintf
            // parr[i] = &strings[i];
        }

        hset    se = hset_init(cnt, VALUE64_FS);
        // TODO: it should be from fs[]!
        int       loaded = hset_loadanyarr(&se, strings, cnt, VALUE64_FS);
        //int     loaded = hset_loadanyarr(&se, parr, cnt, VALUE64_FS);

        fs_alloc_check(false);

        test_validatefree(
            loaded == cnt,
            hset_free(&se),
            "FS from array: loaded %d, expected %d", loaded, cnt
        );
        test_validatefree(
            hset_cnt(&se) == cnt,
            hset_free(&se),
            "FS from array: count = %d, expected %d", hset_cnt(&se), cnt
        );

        // Проверяем наличие всех строк
        for (int i = 0; i < cnt; i++) {
            fs tmp = fscopyf("item_%d", i);
            test_validatefree(
                hset_get(&se, LITERAL64_FS(tmp)),
                hset_free(&se),
                "FS from array: missing 'item_%d'", i
            );
            fsfree(tmp);
        }

        // Освобождаем исходные строки (множество владеет копиями)
        /*for (int i = 0; i < cnt; i++){
            //fstechprint(strings[i] );
            fsfree(strings[i]);
        }*/
        hset_free(&se);
    }
    fs_alloc_check(true);
    /* ---- FS: array with duplicates ---- */
    test_sub("subtest %d: FS from array with duplicates", ++subnum);
    {
        const char *words[] = {"one", "two", "two", "three", "three", "three", "two", "two", "yyyyyy", "two"};
        const int uniq = 4;   // one, two, three, yyyyyy
        fs      strings[COUNT(words)];
        //fs     *parr[COUNT(words)];

        for (int i = 0; i < COUNT(words); i++) {
            strings[i] = fscopy(words[i]);
            //parr[i] = &strings[i];
        }

        hset    se = hset_init(10, VALUE64_FS);
        int     loaded = hset_loadanyarr(&se, strings, COUNT(words), VALUE64_FS);

        // loaded должно быть равно общему числу попыток, но count — только уникальные
        test_validatefree(
            loaded == COUNT(words),
            hset_free(&se),
            "Duplicates: loaded %d, expected %d", loaded, COUNT(words)
        );
        test_validatefree(
            hset_cnt(&se) == uniq,
            hset_free(&se),
            "Duplicates: count = %d, expected %d", hset_cnt(&se), uniq
        );

        //for (int i = 0; i < COUNT(words); i++)
        //    fsfree(strings[i]);
        hset_free(&se);
    }
    fs_alloc_check(true);
    /* ---- FS: array with NULL pointers (empty strings) ---- */
    test_sub("subtest %d: FS array with NULL pointers", ++subnum);
    {
        // Создадим массив, где некоторые указатели NULL (пустые fs)
        fs      arr[5] = { FS(), fscopy("valid1"), fscopy("valid2"), FS(), FS() };

        hset    se = hset_init(10, VALUE64_FS);
        int     loaded = hset_loadanyarr(&se, arr, COUNT(arr), VALUE64_FS);
        logauto(loaded);

        test_validatefree(
            loaded == COUNT(arr),
            hset_free(&se),
            "NULL array: loaded %d, expected 5", loaded
        );
        test_validatefree(
            hset_cnt(&se) == 2, // FS() rejected for now
            hset_free(&se),
            "NULL array: count = %d, expected 3", hset_cnt(&se)
        );
        hset_tech_fprintall(logfile, se);
        // Проверяем, что валидные строки есть
        fs tmp = fscopy("valid1");
        test_validatefree(
            hset_get(&se, LITERAL64_FS(tmp)),
            hset_free(&se),
            "Missing valid1"
        );
        fsfree(tmp);
        tmp = fscopy("valid2");
        test_validatefree(
            hset_get(&se, LITERAL64_FS(tmp)),
            hset_free(&se),
            "Missing valid2"
        );
        fsfree(tmp);

        // iterator musr be here
        for(int i = 0; i < COUNT(arr); i++)
            fsfree(arr[i]);

        hset_free(&se);
    }
    fs_alloc_check(true);
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------- TEST hset_loadfs_str ---------------------------------
static TestStatus
tf_loadfs_str(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    test_sub("subtest %d: load several FS strings", ++subnum);
    {
        hset se = hset_init(10, VALUE64_FS);
        const char *strings[] = {"/tmp/a", "/tmp/b", "/tmp/c", NULL};

        int cnt = hset_loadfs_str(&se, strings);
        test_validatefree(
            cnt == 3 && se.count == 3,
            hset_free(&se),
            "Loaded %d elements, expected 3; set len %d", cnt, se.count
        );
        hset_free(&se);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: load from empty array", ++subnum);
    {
        hset se = hset_init(10, VALUE64_FS);
        const char *empty[] = {NULL};

        int cnt = hset_loadfs_str(&se, empty);
        test_validatefree(
            cnt == 0 && se.count == 0,
            hset_free(&se),
            "Loaded %d elements from empty array, expected 0", cnt
        );
        hset_free(&se);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: load with duplicates", ++subnum);
    {
        hset se = hset_init(10, VALUE64_FS);
        const char *dups[] = {"/tmp/x", "/tmp/y", "/tmp/x", "/tmp/x", "uuuuuu", NULL};

        int cnt = hset_loadfs_str(&se, dups);
        test_validatefree(
            cnt == COUNT(dups) - 1 && se.count == 3,
            hset_free(&se),
            "Loaded %d elements (3 calls), set len %d, expected 2 unique", cnt, se.count
        );
        hset_free(&se);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: NULL set pointer raises SIGINT", ++subnum);
    {
        const char *strings[] = {"/tmp/a", NULL};
        if (!try()) {
            hset_loadfs_str(NULL, strings);
            test_validate(false, "Should have raised SIGINT for NULL set");
        } else {
            logmsg("Exception correctly raised on NULL set");
        }
    }
    fs_alloc_check(true);

    test_sub("subtest %d: NULL strings pointer raises SIGINT", ++subnum);
    {
        hset se = hset_init(10, VALUE64_FS);
        if (!try()) {
            hset_loadfs_str(&se, NULL);
            test_validatefree(
                false, hset_free(&se),
                "Should have raised SIGINT for NULL strings"
            );
        } else {
            hset_free(&se);
            logmsg("Exception correctly raised on NULL strings");
        }
    }
    fs_alloc_check(true);

    test_sub("subtest %d: validate set after load", ++subnum);
    {
        hset se = hset_init(10, VALUE64_FS);
        const char *strings[] = {"/tmp/1", "/tmp/2", NULL};
        hset_loadfs_str(&se, strings);
        test_validatefree(
            hset_validate(stdout, &se),
            hset_free(&se),
            "Set validation failed after load"
        );
        hset_free(&se);
    }
    fs_alloc_check(true);

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 4 ---------------------------------

static TestStatus
tf4(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: empty count", ++subnum);
    {
        hset    se1 = hset_init(100, VALUE64_LNG);
        int     res;
        test_validatefree(
            (res = hset_cnt(&se1) ) == 0, hset_free(&se1), "Must be zero, but not %d", res
        );

        hset_free(&se1);
    }
    test_sub("subtest %d: loaded count", ++subnum);
    {
        rndinit();
        Array   arr = DArray_create(500, ARRAY_ASC);
        hset    se2 = hset_from_dblarr(arr.dv, arr.len);
        int     res;
        // hset_techprint(&se2, 5);
        //Array_print(arr, 5);
        test_validatefree(
            (res = hset_cnt(&se2) ) == Arraylen(arr), (hset_free(&se2), Arrayfree(arr) ), "Must be == %d, but not %d", Arraylen(arr), res
        );
        Arrayfree(arr);
        hset_free(&se2);
    }
    test_sub("subtest %d: loaded count int", ++subnum);
    {
        Array   arr = IArray_create(500, ARRAY_ASC);
        hset    se2 = hset_from_intarr(arr.iv, arr.len);
        int     res;
        //hset_techprint(&se2, 0);
        test_validatefree(
            (res = hset_cnt(&se2) ) == Arraylen(arr), (hset_free(&se2), Arrayfree(arr) ), "Must be == %d, but not %d", Arraylen(arr), res
        );
    test_sub("subtest %d: count after clean", ++subnum);

        hset_clean(&se2);
        test_validatefree(
            (res = hset_cnt(&se2) ) == 0, (hset_free(&se2), Arrayfree(arr) ), "Must be zero after cleanbut not %d", res
        );
        Arrayfree(arr);
        hset_free(&se2);
    }
    // VALUE64_FS
    test_sub("subtest %d: FS empty count", ++subnum);
    {
        hset    se = hset_init(100, VALUE64_FS);
        int     res;
        test_validatefree(
            (res = hset_cnt(&se)) == 0,
            hset_free(&se),
            "FS empty count must be 0, but got %d", res
        );
        hset_free(&se);
    }
    fs_alloc_check(true);
    test_sub("subtest %d: FS loaded count", ++subnum);
    {
        const int cnt = 500;
        fs      strings[cnt];

        for (int i = 0; i < cnt; i++) {
            strings[i] = fscopyf("str_%d", i);   // уникальные строки
            //parr[i] = &strings[i];
        }

        hset    se = hset_init(cnt, VALUE64_FS);
        int     loaded = hset_loadanyarr(&se, strings, cnt, VALUE64_FS);
        int     set_cnt = hset_cnt(&se);

        // Пdосле загрузки исходные строки можно освободить
        //for (int i = 0; i < cnt; i++)
          //  fsfree(strings[i]);

        test_validatefree(
            loaded == cnt && set_cnt == cnt,
            hset_free(&se),
            "FS loaded count: loaded %d, set count %d, expected both %d",
            loaded, set_cnt, cnt
        );
        hset_free(&se);
    }
    fs_alloc_check(true);
    test_sub("subtest %d: FS count after clean", ++subnum);
    {
        const int cnt = 200;
        fs      strings[cnt];
        //fs     *parr[cnt];

        for (int i = 0; i < cnt; i++) {
            strings[i] = fscopyf("clean_%d", i);
            //parr[i] = &strings[i];
        }

        hset    se = hset_init(cnt, VALUE64_FS);
        hset_loadanyarr(&se, strings, cnt, VALUE64_FS);
        for (int i = 0; i < cnt; i++) fsfree(strings[i]);

        hset_clean(&se);
        int  cntclean;

        test_validatefree(
            (cntclean = hset_cnt(&se) ) == 0,
            hset_free(&se),
            "FS count after clean must be 0, but got %d", cntclean
        );
        hset_free(&se);
    }
    fs_alloc_check(true);
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------- TEST 5 ---------------------------------
static TestStatus
tf5(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: clone and compare", ++subnum);
    {
        Array   arr = IArray_create(200, ARRAY_RND);

        hset    se1 = hset_from_intarr(arr.iv, arr.len);
        int     elem = arr.iv[0];   // save one
        Arrayfree(arr);

        hset    se2 = hset_clone(&se1);

        test_validatefree(
            hset_eq(&se1, &se2), (hset_free(&se1), hset_free(&se2) ), "Must be equal!"
        );
        // remove one elem and check again
        test_validatefree(
            hset_del(&se1, LITERAL64_INT(elem) ), (hset_free(&se1), hset_free(&se2) ), "Element %d must exists in se1", elem
        );
        test_validatefree(
            !hset_eq(&se1, &se2), (hset_free(&se1), hset_free(&se2) ), "Must be NOT equal after deleting %d!", elem
        );
        test_validatefree(
            hset_del(&se2, LITERAL64_INT(elem) ), (hset_free(&se1), hset_free(&se2) ), "Element %d must exists in se2", elem
        );
        test_validatefree(
            hset_eq(&se1, &se2), (hset_free(&se1), hset_free(&se2) ), "Must be equal again!"
        );
        hset_free(&se1);
        hset_free(&se2);
    }
    test_sub("subtest %d: compare with different hash table size", ++subnum);
    {
        Array   arr = IArray_create(200, ARRAY_RND);
        // create from array
        hset    se1 = hset_from_intarr(arr.iv, Arraylen(arr) );
        // manually creating, small
        hset    se2 = hset_init(100, VALUE64_INT);
        hset_loadiarr(&se2, arr.iv, Arraylen(arr) );
        test_validatefree(
            hset_eq(&se1, &se2), (hset_free(&se1), hset_free(&se2) ), "Must be equal!"
        );
        // reload se1
        hset_loadiarr(&se1, arr.iv, Arraylen(arr) );
        test_validatefree(
            hset_eq(&se1, &se2), (hset_free(&se1), hset_free(&se2) ), "Must be equal again!"
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    // VALUE64_FS
    test_sub("subtest %d: FS clone and compare", ++subnum);
    {
        const int cnt = 200;
        fs      strings[cnt];
       // fs     *parr[cnt];

        // Создаём массив уникальных строк
        for (int i = 0; i < cnt; i++) {
            strings[i] = fscopyf("fs_str_%d", i);
            //parr[i] = &strings[i];
        }

        hset    se1 = hset_init(cnt, VALUE64_FS);
        hset_loadanyarr(&se1, strings, cnt, VALUE64_FS);

        // Сохраняем первый элемент для последующего удаления
        fs      first_elem = fscopyf("fs_str_%d", 0);   // или просто fscopyf("fs_str_0")
        //fs     *first_parr[1] = { &first_elem };

        // Клонируем se1
        hset    se2 = hset_clone(&se1);

        // 1. Исходно множества должны быть равны
        test_validatefree(
            hset_eq(&se1, &se2),
            (hset_free(&se1), hset_free(&se2), fsfree(first_elem)),
            "FS clone: sets must be equal after clone"
        );

        // 2. Удаляем элемент из se1
        test_validatefree(
            hset_del(&se1, LITERAL64_FS(first_elem)),
            (hset_free(&se1), hset_free(&se2), fsfree(first_elem)),
            "FS clone: failed to delete '%s' from se1", fsstr(first_elem)
        );
        // После удаления множества не должны быть равны
        test_validatefree(
            !hset_eq(&se1, &se2),
            (hset_free(&se1), hset_free(&se2), fsfree(first_elem)),
            "FS clone: sets must NOT be equal after deletion from se1"
        );

        // 3. Удаляем тот же элемент из se2
        test_validatefree(
            hset_del(&se2, LITERAL64_FS(first_elem)),
            (hset_free(&se1), hset_free(&se2), fsfree(first_elem)),
            "FS clone: failed to delete '%s' from se2", fsstr(first_elem)
        );
        // Теперь снова должны быть равны
        test_validatefree(
            hset_eq(&se1, &se2),
            (hset_free(&se1), hset_free(&se2), fsfree(first_elem)),
            "FS clone: sets must be equal after both deletions"
        );

        // Освобождаем исходные строки (множества владеют копиями)
        //for (int i = 0; i < cnt; i++)
          //  fsfree(strings[i]);
        fsfree(first_elem);
        hset_free(&se1);
        hset_free(&se2);
    }
    fs_alloc_check(true);
    test_sub("subtest %d: FS compare with different hash table size", ++subnum);
    {
        const int cnt = 200;
        fs      strings[cnt];
        //fs     *parr[cnt];

        for (int i = 0; i < cnt; i++) {
            strings[i] = fscopyf("diff_%d", i);
            //parr[i] = &strings[i];
        }

        // Первое множество – стандартное создание из массива
        hset    se1 = hset_init(cnt, VALUE64_FS);
        hset_loadanyarr(&se1, strings, cnt, VALUE64_FS);

        for (int i = 0; i < cnt; i++)
            strings[i] = fscopyf("diff_%d", i);

        // Второе – с заведомо меньшим начальным размером таблицы
        hset    se2 = hset_init(cnt / 2, VALUE64_FS);
        hset_loadanyarr(&se2, strings, cnt, VALUE64_FS);

        // Они должны быть равны, несмотря на разный размер таблиц
        test_validatefree(
            hset_eq(&se1, &se2),
            (hset_free(&se1), hset_free(&se2)),
            "FS diff size: sets must be equal after initial load"
        );

        for (int i = 0; i < cnt; i++)
            strings[i] = fscopyf("diff_%d", i);

        // Повторная загрузка тех же данных не должна испортить равенство
        hset_loadanyarr(&se1, strings, cnt, VALUE64_FS);
        test_validatefree(
            hset_eq(&se1, &se2),
            (hset_free(&se1), hset_free(&se2)),
            "FS diff size: sets must be equal after reloading duplicates"
        );

        //for (int i = 0; i < cnt; i++)
          //  fsfree(strings[i]);
        hset_free(&se1);
        hset_free(&se2);
    }
    fs_alloc_check(true);
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------- TEST 6 ---------------------------------
static TestStatus
tf6(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: clone and !=", ++subnum);
    {
        Array   arr = IArray_create(200, ARRAY_RND);

        hset    se1 = hset_from_intarr(arr.iv, arr.len);
        int     elem = arr.iv[0];   // save one
        Arrayfree(arr);

        hset    se2 = hset_clone(&se1);

        test_validatefree(
            hset_noteq(&se1, &se2) == false, (hset_free(&se1), hset_free(&se2) ), "Must be equal (not equalt must return false)!"
        );
        // remove one elem and check again
        test_validatefree(
            hset_del(&se1, LITERAL64_INT(elem) ), (hset_free(&se1), hset_free(&se2) ), "Element %d must exists in se1", elem
        );
        test_validatefree(
            hset_noteq(&se1, &se2), (hset_free(&se1), hset_free(&se2) ), "Must be NOT equal (returns true) after deleting %d!", elem
        );
        test_validatefree(
            hset_del(&se2, LITERAL64_INT(elem) ), (hset_free(&se1), hset_free(&se2) ), "Element %d must exists in se2", elem
        );
        test_validatefree(
            hset_noteq(&se1, &se2) == false, (hset_free(&se1), hset_free(&se2) ), "Must be equal again (returns false)!"
        );
        hset_free(&se1);
        hset_free(&se2);
    }
    test_sub("subtest %d: != with different hash table size", ++subnum);
    {
        Array   arr = IArray_create(150, ARRAY_RND);
        int     elem = arr.iv[0];   // save one
        // create from array
        hset    se1 = hset_from_intarr(arr.iv, Arraylen(arr) );
        // manually creating, small
        hset    se2 = hset_init(50, VALUE64_INT);
        hset_loadiarr(&se2, arr.iv, Arraylen(arr) );
        test_validatefree(
            hset_noteq(&se1, &se2) == false, (hset_free(&se1), hset_free(&se2) ), "Must be equal (returns false)!"
        );
        // reload se1
        hset_loadiarr(&se1, arr.iv, Arraylen(arr) );
        test_validatefree(
            hset_noteq(&se1, &se2) == false, (hset_free(&se1), hset_free(&se2) ), "Must be equal again (return false)!"
        );
        test_validatefree(
            hset_del(&se1, LITERAL64_INT(elem) ), (hset_free(&se1), hset_free(&se2) ), "Element %d must exists in se1", elem
        );
        test_validatefree(
            hset_noteq(&se1, &se2), (hset_free(&se1), hset_free(&se2) ), "Must be not equal after deleting %d (return tur)!", elem
        );
        test_validatefree(
            hset_del(&se2, LITERAL64_INT(elem) ), (hset_free(&se1), hset_free(&se2) ), "Element %d must exists in se2", elem
        );
        test_validatefree(
            hset_noteq(&se1, &se2) == false, (hset_free(&se1), hset_free(&se2) ),
            "Must be equal again after deleting %d from se2 (return false)!", elem
        );

        hset_free(&se1);
        hset_free(&se2);
    }
    fs_alloc_check(true);
    // VALUE64_FS
    test_sub("subtest %d: FS clone and !=", ++subnum);
    {
        const int cnt = 10;
        fs      strings[cnt];

        // Создаём массив уникальных строк
        for (int i = 0; i < cnt; i++) {
            strings[i] = fscopyf("fs_str_%d", i);
        }

        hset    se1 = hset_init(cnt, VALUE64_FS);
        hset_loadanyarr(&se1, strings, cnt, VALUE64_FS);

        // Клонируем se1
        hset    se2 = hset_clone(&se1);

        // 1. Исходно множества должны быть равны (hset_noteq возвращает false)
        test_validatefree(
            hset_noteq(&se1, &se2) == false,
            (hset_free(&se1), hset_free(&se2)),
            "FS clone !=: sets must be equal initially"
        );

        // Удаляем первый элемент из se1 (сохраняем ссылку на него для проверки)
        fs      first_elem = fscopyf("fs_str_0");
        test_validatefree(
            hset_del(&se1, LITERAL64_FS(first_elem)),
            (hset_free(&se1), hset_free(&se2), fsfree(first_elem)),
            "FS clone !=: failed to delete first element from se1"
        );

        // 2. После удаления множества должны быть НЕ равны
        test_validatefree(
            hset_noteq(&se1, &se2) == true,
            (hset_free(&se1), hset_free(&se2), fsfree(first_elem)),
            "FS clone !=: sets must be not equal after deletion from se1"
        );

        // Удаляем тот же элемент из se2
        test_validatefree(
            hset_del(&se2, LITERAL64_FS(first_elem)),
            (hset_free(&se1), hset_free(&se2), fsfree(first_elem)),
            "FS clone !=: failed to delete first element from se2"
        );

        // 3. Теперь снова должны быть равны
        test_validatefree(
            hset_noteq(&se1, &se2) == false,
            (hset_free(&se1), hset_free(&se2), fsfree(first_elem)),
            "FS clone !=: sets must be equal after both deletions"
        );

        // Освобождаем исходные строки (множества владеют копиями)
        //for (int i = 0; i < cnt; i++)
          //  fsfree(strings[i]);

        fsfree(first_elem);
        hset_free(&se1);
        hset_free(&se2);
    }
    fs_alloc_check(true);
    test_sub("subtest %d: FS != with different hash table size", ++subnum);
    {
        const int cnt = 15;
        fs      strings[cnt];

        for (int i = 0; i < cnt; i++)
            strings[i] = fscopyf("diff_fs_%d", i);

        // Первое множество – создание из массива
        hset    se1 = hset_init(cnt, VALUE64_FS);
        hset_loadanyarr(&se1, strings, cnt, VALUE64_FS);

        // Второе – с заведомо меньшим размером таблицы
        hset    se2 = hset_init(cnt / 3, VALUE64_FS);
        for (int i = 0; i < cnt; i++)
            strings[i] = fscopyf("diff_fs_%d", i);
        hset_loadanyarr(&se2, strings, cnt, VALUE64_FS);

        // 1. Должны быть равны, несмотря на разный размер таблиц
        test_validatefree(
            hset_noteq(&se1, &se2) == false,
            (hset_free(&se1), hset_free(&se2)),
            "FS diff size !=: sets must be equal initially"
        );

        // Повторная загрузка тех же данных не должна испортить равенство
        for (int i = 0; i < cnt; i++)
            strings[i] = fscopyf("diff_fs_%d", i);
        hset_loadanyarr(&se1, strings, cnt, VALUE64_FS);
        test_validatefree(
            hset_noteq(&se1, &se2) == false,
            (hset_free(&se1), hset_free(&se2)),
            "FS diff size !=: sets must be equal after reloading duplicates"
        );

        // Удаляем первый элемент из se1
        fs      first_elem = fscopyf("diff_fs_0");
        test_validatefree(
            hset_del(&se1, LITERAL64_FS(first_elem)),
            (hset_free(&se1), hset_free(&se2), fsfree(first_elem)),
            "FS diff size !=: failed to delete first element from se1"
        );

        // 2. После удаления должны стать не равны
        test_validatefree(
            hset_noteq(&se1, &se2) == true,
            (hset_free(&se1), hset_free(&se2), fsfree(first_elem)),
            "FS diff size !=: sets must be not equal after deletion from se1"
        );
        // Удаляем тот же элемент из se2
        test_validatefree(
            hset_del(&se2, LITERAL64_FS(first_elem)),
            (hset_free(&se1), hset_free(&se2), fsfree(first_elem)),
            "FS diff size !=: failed to delete first element from se2"
        );
       // fs_alloc_check(true);
        // 3. Снова равны
        test_validatefree(
            hset_noteq(&se1, &se2) == false,
            (hset_free(&se1), hset_free(&se2), fsfree(first_elem)),
            "FS diff size !=: sets must be equal after both deletions"
        );

        fsfree(first_elem);

        hset_free(&se1);
        hset_free(&se2);
    }
    fs_alloc_check(true);
    /* Бонус: проверка неравенства после перемещения (VALUE64_FSMOVE) */
    test_sub("subtest %d: FS != after move", ++subnum);
    {
        hset    se1 = hset_init(10, VALUE64_FS);
        hset    se2 = hset_init(10, VALUE64_FS);

        // Вставляем одну и ту же строку, но в se1 через перемещение, в se2 через копирование
        fs      orig = fscopyf("move_vs_copy");
        hset_set(&se1, value64_movefs(&orig));          // orig опустеет

        hset_set(&se2, LITERAL64_FS(orig));          // orig пуст, поэтому вставится пустая строка? Нет, VALUE64_FS передаёт &orig, а orig.v == NULL – будет пустая строка.

        // Чтобы избежать путаницы, создадим новую строку для se2
        fs      copy = fscopyf("move_vs_copy");
        hset_set(&se2, LITERAL64_FS(copy));

        // Множества должны быть равны, потому что строки одинаковы
        test_validatefree(
            hset_noteq(&se1, &se2) == false,
            (hset_free(&se1), hset_free(&se2), fsfree(copy)),
            "FS != after move: sets with moved and copied strings must be equal"
        );

        // Удаляем элемент из se1
        test_validatefree(
            hset_del(&se1, LITERAL64_FS(copy)),
            (hset_free(&se1), hset_free(&se2), fsfree(copy)),
            "FS != after move: failed to delete from se1"
        );
        // Теперь должны быть не равны
        test_validatefree(
            hset_noteq(&se1, &se2) == true,
            (hset_free(&se1), hset_free(&se2), fsfree(copy)),
            "FS != after move: sets must be not equal after deletion"
        );
        fs_fprint_checker_cnt(stdout, "V");

        fsfree(copy);
        hset_tech_printall(se1);
        hset_tech_printall(se2);

        hset_free(&se1);
        hset_free(&se2);
    }
    fs_alloc_check(true);
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------- TEST 7 ---------------------------------
static TestStatus
tf7(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: cloneas int -> long", ++subnum);
    {
        Array   arr = IArray_create(/*200*/ 10, ARRAY_ASC);

        hset    se1 = hset_from_intarr(arr.iv, arr.len);
        Arrayfree(arr);

        hset    se2 = hset_cloneas(&se1, VALUE64_LNG);

        test_validatefree(
            hset_validate(stdout, &se1) && hset_validate(stdout, &se2),
            (hset_free(&se1), hset_free(&se2) ), "Validation failed"
        );
        int     sz1 = se1.sz, sz2 = se2.sz;
        test_validatefree(
            se1.sz == se2.sz, (hset_free(&se1), hset_free(&se2) ), "Must have equal table size %d : %d", sz1, sz2
        );

        sz1 = hset_cnt(&se1);
        sz2 = hset_cnt(&se2);
        test_validatefree(
            se1.sz == se2.sz, (hset_free(&se1), hset_free(&se2) ), "Must have equal element count %d : %d", sz1, sz2
        );

        // compare directly!
        for (int i = 0; i < se1.sz; i++){
            const hset_elem *el = se1.table[i];
            while (el){
                int val = el->v.ival;
                test_validatefree(
                    hset_get(&se2, LITERAL64_LNG(val) ), (hset_free(&se1), hset_free(&se2) ), "Unable to find elem %d in se2", val
                );
                el = el->next;
            }
        }
        hset_free(&se1);
        hset_free(&se2);
    }
    test_sub("subtest %d: cloneas int -> double", ++subnum);
    {
        Array   arr = IArray_create(110, ARRAY_ASC);

        hset    se1 = hset_from_intarr(arr.iv, arr.len);
        Arrayfree(arr);

        hset    se2 = hset_cloneas(&se1, VALUE64_DBL);

        int     sz1 = se1.sz, sz2 = se2.sz;
        test_validatefree(
            se1.sz == se2.sz, (hset_free(&se1), hset_free(&se2) ), "Must have equal table size %d : %d", sz1, sz2
        );

        test_validatefree(
            hset_validate(stdout, &se1) && hset_validate(stdout, &se2),
            (hset_free(&se1), hset_free(&se2)), "Validation failed se1"
        );

        sz1 = hset_cnt(&se1);
        sz2 = hset_cnt(&se2);
        test_validatefree(
            se1.sz == se2.sz, (hset_free(&se1), hset_free(&se2) ), "Must have equal element count %d : %d", sz1, sz2
        );

        // compare directly! is it correct to compare int vs double?
        for (int i = 0; i < se1.sz; i++){
            const hset_elem *el = se1.table[i];
            while (el){
                int val = el->v.ival;
                test_validatefree(
                    hset_get(&se2, LITERAL64_DBL(val) ), (hset_free(&se1), hset_free(&se2) ), "Unable to find elem %d in se2", val
                );
                el = el->next;
            }
        }
        hset_free(&se1);
        hset_free(&se2);
    }
    /* ========== 1. cloneas int -> FS ========== */
    test_sub("subtest %d: cloneas int -> FS", ++subnum);
    {
        int     cnt = 10 /*200*/;
        Array   arr = IArray_create(cnt, ARRAY_ASC);   // 0,1,2,...,199
        hset    se_int = hset_from_intarr(arr.iv, arr.len);
        Arrayfree(arr);

        hset    se_fs = hset_cloneas(&se_int, VALUE64_FS);

        test_validatefree(
            hset_validate(stdout, &se_int) && hset_validate(stdout, &se_fs),
            (hset_free(&se_int), hset_free(&se_fs)),
            "Validation failed after int->FS clone"
        );

        int     sz1, sz2;
        test_validatefree(
            (sz1 = se_int.sz) == (sz2 = se_fs.sz),
            (hset_free(&se_int), hset_free(&se_fs)),
            "Table sizes differ: %d vs %d", sz1, sz2
        );

        int     cnt1 = hset_cnt(&se_int), cnt2 = hset_cnt(&se_fs);
        test_validatefree(
            (cnt1 = hset_cnt(&se_int) ) == (cnt2 = hset_cnt(&se_fs) ),
            (hset_free(&se_int), hset_free(&se_fs)),
            "Counts differ: %d vs %d", cnt1, cnt2
        );

        fs tmp = FS();
        HSET_FOREACH_INT(&se_int, ival) {
            fs_sprintf(&tmp, "%d", ival);
            test_validatefree(
                hset_get(&se_fs, LITERAL64_FS(tmp)),
                (hset_free(&se_int), hset_free(&se_fs), fsfree(tmp)),
                "FS set missing '%d'", ival
            );
        }
        fsfree(tmp);

        hset_free(&se_int);
        hset_free(&se_fs);
    }
    fs_alloc_check(true);
    /* ========== 2. cloneas FS -> int ========== */
    test_sub("subtest %d: cloneas FS -> int", ++subnum);
    {
        const int   cnt = 50;
        fs          strings[cnt];

        for (int i = 0; i < cnt; i++) {
            strings[i] = fscopyf("%d", i);
        }

        hset    se_fs = hset_init(cnt, VALUE64_FS);
        hset_loadanyarr(&se_fs, strings, cnt, VALUE64_FS);

        hset    se_int = hset_cloneas(&se_fs, VALUE64_INT);

        test_validatefree(
            hset_validate(stdout, &se_fs) && hset_validate(stdout, &se_int),
            (hset_free(&se_fs), hset_free(&se_int)),
            "Validation failed after FS->int clone"
        );

        int         cnt1, cnt2;
        test_validatefree(
            (cnt1 = hset_cnt(&se_fs) ) == (cnt2 = hset_cnt(&se_fs) ),
            (hset_free(&se_fs), hset_free(&se_int)),
            "Counts differ: %d vs %d", cnt1, cnt2
        );

        for (int i = 0; i < cnt; i++) {
            test_validatefree(
                hset_get(&se_int, LITERAL64_INT(i)),
                (hset_free(&se_fs), hset_free(&se_int)),
                "Int set missing %d", i
            );
        }

        hset_free(&se_fs);
        hset_free(&se_int);
    }
    fs_alloc_check(true);
    /* ========== 3. cloneas FS -> FS (identity) ========== */
    test_sub("subtest %d: cloneas FS -> FS (identity)", ++subnum);
    {
        const int cnt = 5;
        fs      strings[cnt];

        for (int i = 0; i < cnt; i++) {
            strings[i] = fscopyf("str_%d", i);
        }

        hset    se1 = hset_init(cnt, VALUE64_FS);
        hset_loadanyarr(&se1, strings, cnt, VALUE64_FS);

        hset    se2 = hset_cloneas(&se1, VALUE64_FS);

        test_validatefree(
            hset_validate(stdout, &se1) && hset_validate(stdout, &se2),
            (hset_free(&se1), hset_free(&se2)),
            "Validation failed after FS->FS clone"
        );

        fs      tmp = FS();
        for (int i = 0; i < cnt; i++) {
            fs_sprintf(&tmp, "str_%d", i);
            test_validatefree(
                hset_get(&se2, LITERAL64_FS(tmp)),
                (hset_free(&se1), hset_free(&se2), fsfree(tmp)),
                "Clone missing 'str_%d'", i
            );
        }
        fsfree(tmp);

        hset_free(&se1);
        hset_free(&se2);
    }
    fs_alloc_check(true);
    /* ========== 4. cloneas double -> FS ========== */
    test_sub("subtest %d: cloneas double -> FS", ++subnum);
    {
        const int cnt = 30;
        double  vals[cnt];
        // TODO: rework via DArray_create
        for (int i = 0; i < cnt; i++)
            vals[i] = i * 1.5;

        hset    se_dbl = hset_init(cnt, VALUE64_DBL);
        for (int i = 0; i < cnt; i++)
            hset_set(&se_dbl, LITERAL64_DBL(vals[i]));

        hset    se_fs = hset_cloneas(&se_dbl, VALUE64_FS);

        test_validatefree(
            hset_validate(stdout, &se_dbl) && hset_validate(stdout, &se_fs),
            (hset_free(&se_dbl), hset_free(&se_fs)),
            "Validation failed after double->FS clone"
        );

        fs      tmp = FS();
        for (int i = 0; i < cnt; i++) {
            fs_sprintf(&tmp, "%g", vals[i]);
            test_validatefree(
                hset_get(&se_fs, LITERAL64_FS(tmp)),
                (hset_free(&se_dbl), hset_free(&se_fs), fsfree(tmp)),
                "FS set missing '%g'", vals[i]
            );
        }
        fsfree(tmp);

        hset_free(&se_dbl);
        hset_free(&se_fs);
    }
    fs_alloc_check(true);
    /* ========== 5. cloneas FS -> double ========== */
    test_sub("subtest %d: cloneas FS -> double", ++subnum);
    {
        const int cnt = 30;
        double  vals[cnt];
        fs      strings[cnt];

        // TODO: replace to macro DArray_create( {iter * 1.5} )
        for (int i = 0; i < cnt; i++) {
            vals[i] = i * 1.5;
            strings[i] = fscopyf("%g", vals[i]);
        }

        hset    se_fs = hset_init(cnt, VALUE64_FS);
        hset_loadanyarr(&se_fs, strings, cnt, VALUE64_FS);
        for (int i = 0; i < cnt; i++)
            fsfree(strings[i]);

        hset    se_dbl = hset_cloneas(&se_fs, VALUE64_DBL);

        test_validatefree(
            hset_validate(stdout, &se_fs) && hset_validate(stdout, &se_dbl),
            (hset_free(&se_fs), hset_free(&se_dbl)),
            "Validation failed after FS->double clone"
        );

        int     cnt1, cnt2;
        test_validatefree(
            (cnt1 = hset_cnt(&se_fs) ) == (cnt2 = hset_cnt(&se_dbl) ),
            (hset_free(&se_fs), hset_free(&se_dbl)),
            "Counts differ: %d vs %d", cnt1, cnt2
        );

        for (int i = 0; i < cnt; i++) {
            test_validatefree(
                hset_get(&se_dbl, LITERAL64_DBL(vals[i])),
                (hset_free(&se_fs), hset_free(&se_dbl)),
                "Double set missing %g", vals[i]
            );
        }

        hset_free(&se_fs);
        hset_free(&se_dbl);
    }
    fs_alloc_check(true);
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------- TEST 8 ---------------------------------
static TestStatus
tf8(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    test_sub("subtest %d: empty in empty", ++subnum);
    {
        hset empty1 = hset_init(10, VALUE64_INT);
        hset empty2 = hset_init(100, VALUE64_INT);

        test_validatefree(
            hset_validate(stdout, &empty1) && hset_validate(stdout, &empty1),
            (hset_free(&empty1), hset_free(&empty2) ), "Validation failed empty"
        );
        test_validatefree(
            hset_in(&empty1, &empty2), (hset_free(&empty1), hset_free(&empty2) ),
            "Empty set should be subset of empty set"
        );
        hset_free(&empty1);
        hset_free(&empty2);
    }
    test_sub("subtest %d: empty in nonempty", ++subnum);
    {
        int vals[] = {1, 3, 5, 7, 9};
        hset empty    = hset_init(10, VALUE64_INT);
        hset nonempty = hset_from_intarr(vals, COUNT(vals) );

        test_validatefree(
            hset_validate(stdout, &empty) && hset_validate(stdout, &nonempty),
            (hset_free(&empty), hset_free(&nonempty) ), "Validation failed empty"
        );
        test_validatefree(
            hset_in(&empty, &nonempty), (hset_free(&empty), hset_free(&nonempty) ),
            "Empty set should be subset of any set"
        );
        hset_free(&empty);
        hset_free(&nonempty);
    }
    test_sub("subtest %d: nonempty not in empty", ++subnum);
    {
        int vals[] = {1, 3, 5, 7, 9};
        hset empty    = hset_init(10, VALUE64_INT);
        hset nonempty = hset_from_intarr(vals, COUNT(vals) );

        test_validatefree(
            !hset_in(&nonempty, &empty), (hset_free(&empty), hset_free(&nonempty) ),
            "Non-empty set should NOT be subset of empty set"
        );
        hset_free(&empty);
        hset_free(&nonempty);
    }
    test_sub("subtest %d: equal sets", ++subnum);
    {
        int all_vals[] = {1, 3, 5, 7, 9};

        hset superset = hset_from_intarr(all_vals, COUNT(all_vals) );
        hset subset   = hset_from_intarr(all_vals, COUNT(all_vals) );

        test_validatefree(
            hset_in(&subset, &superset), (hset_free(&superset), hset_free(&subset) ),
            "Subset should be strict in equal set"
        );
        hset_free(&superset);
        hset_free(&subset);
    }
    test_sub("subtest %d: subset in superset", ++subnum);
    {
        int all_vals[] = {1, 3, 5, 7, 9};
        int sub_vals[] = {1, 5, 9};

        hset superset = hset_from_intarr(all_vals, COUNT(all_vals) );
        hset subset   = hset_from_intarr(sub_vals, COUNT(sub_vals) );

        test_validatefree(
            hset_in(&subset, &superset), (hset_free(&superset), hset_free(&subset) ),
            "Subset should be in superset"
        );
        hset_free(&superset);
        hset_free(&subset);
    }

    test_sub("subtest %d: superset not in subset", ++subnum);
    {
        int all_vals[] = {1, 3, 5, 7, 9};
        int sub_vals[] = {1, 5, 9};

        hset superset = hset_from_intarr(all_vals, COUNT(all_vals) );
        hset subset   = hset_from_intarr(sub_vals, COUNT(sub_vals) );

        test_validatefree(
            !hset_in(&superset, &subset), (hset_free(&superset), hset_free(&subset) ),
            "Superset should NOT be in subset"
        );
        hset_free(&superset);
        hset_free(&subset);
    }

    test_sub("subtest %d: type mismatch raise SIGINT", ++subnum);
    {
        int int_vals[] = {1, 2, 3};
        hset int_set = hset_from_intarr(int_vals, 3);
        hset dbl_set = hset_init(10, VALUE64_DBL);
        hset_set(&dbl_set, LITERAL64_DBL(1.0));
        hset_set(&dbl_set, LITERAL64_DBL(2.0));
        if (!try () ){
            test_validatefree(
                !hset_in(&int_set, &dbl_set),
                (hset_free(&int_set), hset_free(&dbl_set)),
                "Different types should never be subset"
            );
        } else {
            //err_printstacktrace();
            hset_free(&int_set);
            hset_free(&dbl_set);
            return logret(TEST_PASSED, "done");
        }
        return logret(TEST_FAILED, "done");
    }
    /* // fs
    test_sub("subtest %d: empty FS in empty FS", ++subnum);
    {
        hset empty1 = hset_init(10, VALUE64_FS);
        hset empty2 = hset_init(100, VALUE64_FS);

        test_validatefree(
            hset_validate(stdout, &empty1) && hset_validate(stdout, &empty2),
            (hset_free(&empty1), hset_free(&empty2) ),
            "Validation failed empty FS"
        );
        test_validatefree(
            hset_in(&empty1, &empty2), (hset_free(&empty1), hset_free(&empty2) ),
            "Empty FS set should be subset of empty FS set"
        );
        hset_free(&empty1);
        hset_free(&empty2);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: empty FS in nonempty FS", ++subnum);
    {
        hset empty    = hset_init(10, VALUE64_FS);
        hset nonempty = hset_init(10, VALUE64_FS);

        hset_loadfs_str(&nonempty, (char *[]){"/tmp/a", "/tmp/b", NULL} );

        test_validatefree(
            hset_validate(stdout, &empty) && hset_validate(stdout, &nonempty),
            (hset_free(&empty), hset_free(&nonempty) ),
            "Validation failed empty FS vs nonempty FS"
        );
        test_validatefree(
            hset_in(&empty, &nonempty), (hset_free(&empty), hset_free(&nonempty) ),
            "Empty FS set should be subset of any FS set"
        );
        hset_free(&empty);
        hset_free(&nonempty);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: nonempty FS not in empty FS", ++subnum);
    {
        hset empty    = hset_init(10, VALUE64_FS);
        hset nonempty = hset_init(10, VALUE64_FS);

        hset_loadfs_str(&nonempty, (char *[]){"/tmp/x", NULL} );
        // hset_set(&nonempty, value64_createfs("/tmp/x"));

        test_validatefree(
            !hset_in(&nonempty, &empty), (hset_free(&empty), hset_free(&nonempty) ),
            "Non-empty FS set should NOT be subset of empty FS set"
        );
        hset_free(&empty);
        hset_free(&nonempty);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: equal FS sets", ++subnum);
    {
        hset superset = hset_init(10, VALUE64_FS);
        hset subset   = hset_init(10, VALUE64_FS);

        value64 f1 = value64_createfs("/tmp/1");
        value64 f2 = value64_createfs("/tmp/2");
        hset_set(&superset, f1);
        hset_set(&superset, f2);
        hset_set(&subset,   f1);
        hset_set(&subset,   f2);

        test_validatefree(
            hset_in(&subset, &superset), (hset_free(&superset), hset_free(&subset) ),
            "Equal FS sets: subset should be in superset"
        );
        hset_free(&superset);
        hset_free(&subset);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: FS subset in FS superset", ++subnum);
    {
        hset superset = hset_init(10, VALUE64_FS);
        hset subset   = hset_init(10, VALUE64_FS);

        value64 f1 = value64_createfs("/tmp/a");
        value64 f2 = value64_createfs("/tmp/b");
        value64 f3 = value64_createfs("/tmp/c");
        hset_set(&superset, f1);
        hset_set(&superset, f2);
        hset_set(&superset, f3);

        hset_set(&subset, f1);
        hset_set(&subset, f3);

        test_validatefree(
            hset_in(&subset, &superset), (hset_free(&superset), hset_free(&subset) ),
            "Subset should be in superset"
        );
        hset_free(&superset);
        hset_free(&subset);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: FS superset not in FS subset", ++subnum);
    {
        hset superset = hset_init(10, VALUE64_FS);
        hset subset   = hset_init(10, VALUE64_FS);

        value64 f1 = value64_createfs("/tmp/a");
        value64 f2 = value64_createfs("/tmp/b");
        value64 f3 = value64_createfs("/tmp/c");
        hset_set(&superset, f1);
        hset_set(&superset, f2);
        hset_set(&superset, f3);

        hset_set(&subset, f1);
        hset_set(&subset, f3);

        test_validatefree(
            !hset_in(&superset, &subset), (hset_free(&superset), hset_free(&subset) ),
            "Superset should NOT be in subset"
        );
        hset_free(&superset);
        hset_free(&subset);
    }
    fs_alloc_check(true);

    test_sub("subtest %d: FS vs INT type mismatch raise SIGINT", ++subnum);
    {
        hset fs_set  = hset_init(10, VALUE64_FS);
        hset int_set = hset_init(10, VALUE64_INT);
        hset_set(&fs_set, value64_createfs("/tmp/z"));
        hset_set(&int_set, LITERAL64_INT(42));

        if (!try()) {
            test_validatefree(
                !hset_in(&fs_set, &int_set),
                (hset_free(&fs_set), hset_free(&int_set)),
                "FS set should not be subset of INT set (type mismatch)"
            );
        } else {
            hset_free(&fs_set);
            hset_free(&int_set);
            return logret(TEST_PASSED, "done");
        }
        return logret(TEST_FAILED, "done");
    }
    fs_alloc_check(true);
    */
    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 9 ---------------------------------
static TestStatus
tf9(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    test_sub("subtest %d: empty strict in empty", ++subnum);
    {
        hset empty1    = hset_init(10, VALUE64_INT);
        hset empty2 = hset_init(100, VALUE64_INT);

        test_validatefree(
            hset_validate(stdout, &empty1) && hset_validate(stdout, &empty1),
            (hset_free(&empty1), hset_free(&empty2) ), "Validation failed empty"
        );
        test_validatefree(
            hset_strictin(&empty1, &empty2) == false, (hset_free(&empty1), hset_free(&empty2) ),
            "Empty set should NOT be subset of empty set"
        );
        hset_free(&empty1);
        hset_free(&empty2);
    }
    test_sub("subtest %d: empty strict in nonempty", ++subnum);
    {
        int vals[] = {1, 3, 5, 7, 9};
        hset empty    = hset_init(10, VALUE64_INT);
        hset nonempty = hset_from_intarr(vals, COUNT(vals) );

        test_validatefree(
            hset_validate(stdout, &empty) && hset_validate(stdout, &nonempty),
            (hset_free(&empty), hset_free(&nonempty) ), "Validation failed empty"
        );
        test_validatefree(
            hset_strictin(&empty, &nonempty), (hset_free(&empty), hset_free(&nonempty) ),
            "Empty set should be strict subset of any set"
        );
        hset_free(&empty);
        hset_free(&nonempty);
    }
    test_sub("subtest %d: nonempty not in empty", ++subnum);
    {
        int vals[] = {1, 3, 5, 7, 9};
        hset empty    = hset_init(10, VALUE64_INT);
        hset nonempty = hset_from_intarr(vals, COUNT(vals) );

        test_validatefree(
            !hset_strictin(&nonempty, &empty), (hset_free(&empty), hset_free(&nonempty) ),
            "Non-empty set should NOT be strict subset of empty set"
        );
        hset_free(&empty);
        hset_free(&nonempty);
    }

    test_sub("subtest %d: subset in superset", ++subnum);
    {
        int all_vals[] = {1, 3, 5, 7, 9};
        int sub_vals[] = {1, 5, 9};

        hset superset = hset_from_intarr(all_vals, COUNT(all_vals) );
        hset subset = hset_from_intarr(sub_vals, COUNT(sub_vals) );

        test_validatefree(
            hset_strictin(&subset, &superset), (hset_free(&superset), hset_free(&subset) ),
            "Subset should be strict in superset"
        );
        hset_free(&superset);
        hset_free(&subset);
    }
    test_sub("subtest %d: equal sets", ++subnum);
    {
        int all_vals[] = {1, 3, 5, 7, 9};

        hset superset = hset_from_intarr(all_vals, COUNT(all_vals) );
        hset subset   = hset_from_intarr(all_vals, COUNT(all_vals) );

        test_validatefree(
            hset_strictin(&subset, &superset) == false, (hset_free(&superset), hset_free(&subset) ),
            "Subset should NOT be strict in equal set"
        );
        hset_free(&superset);
        hset_free(&subset);
    }
    test_sub("subtest %d: superset not in subset", ++subnum);
    {
        int all_vals[] = {1, 3, 5, 7, 9};
        int sub_vals[] = {1, 5, 9};

        hset superset = hset_from_intarr(all_vals, COUNT(all_vals) );
        hset subset = hset_from_intarr(sub_vals, COUNT(sub_vals) );

        test_validatefree(
            !hset_strictin(&superset, &subset), (hset_free(&superset), hset_free(&subset) ),
            "Superset should NOT be strict in subset"
        );
        hset_free(&superset);
        hset_free(&subset);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 10 ---------------------------------
static TestStatus
tf10(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Пустое минус пустое */
    test_sub("subtest %d: empty minus empty", ++subnum);
    {
        hset    se1 = hset_init(10, VALUE64_INT);
        hset    se2 = hset_init(10, VALUE64_INT);
        hset   *res = hset_minus(&se1, &se2);

        int     ok = (res == &se1) && (hset_cnt(&se1) == 0);
        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Empty minus empty: res=%p, cnt=%d (expected res=&se1, cnt=0)",
            (void*)res, hset_cnt(&se1)
        );
        hset_free(&se1);
        hset_free(&se2);
    }
    /* 2. Пустое минус непустое */
    test_sub("subtest %d: empty minus non-empty", ++subnum);
    {
        hset    empty = hset_init(10, VALUE64_INT);
        int     vals[] = {1, 2, 3};
        hset    nonempty = hset_from_intarr(vals, COUNT(vals));
        hset   *res = hset_minus(&empty, &nonempty);

        int     ok = (res == &empty) && (hset_cnt(&empty) == 0) &&
                     (hset_cnt(&nonempty) == COUNT(vals));
        test_validatefree(
            ok,
            (hset_free(&empty), hset_free(&nonempty)),
            "Empty minus non-empty: res=%p, cnt=%d (expected res=&empty, cnt=0)",
            (void*)res, hset_cnt(&empty)
        );
        hset_free(&empty);
        hset_free(&nonempty);
    }
    /* 3. Непустое минус пустое */
    test_sub("subtest %d: non-empty minus empty", ++subnum);
    {
        int     vals[] = {10, 20, 30};
        hset    se1 = hset_from_intarr(vals, COUNT(vals));
        hset    empty = hset_init(10, VALUE64_INT);
        int     cnt_before = hset_cnt(&se1);
        hset   *res = hset_minus(&se1, &empty);

        int     ok = (res == &se1) && (hset_cnt(&se1) == cnt_before);
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&se1, LITERAL64_INT(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&empty)),
            "Non-empty minus empty: res=%p, cnt=%d (expected res=&se1, cnt=%d)",
            (void*)res, hset_cnt(&se1), cnt_before
        );
        hset_free(&se1);
        hset_free(&empty);
    }

    /* 4. Множество минус его копия (все элементы должны быть удалены) */
    test_sub("subtest %d: set minus its copy", ++subnum);
    {
        int     vals[] = {5, 10, 15, 20};
        hset    se1 = hset_from_intarr(vals, COUNT(vals));
        hset    se2 = hset_from_intarr(vals, COUNT(vals));
        /*hset    se2 = hset_init(10, VALUE64_INT);
        for (int i = 0; i < COUNT(vals); i++)
            hset_set(&se2, LITERAL64_INT(vals[i])); */

        int     cnt_before = hset_cnt(&se1);
        hset   *res = hset_minus(&se1, &se2);

        int     ok = (res == &se1) && (hset_cnt(&se1) == 0);
        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Set minus its copy: res=%p, cnt=%d (expected res=&se1, cnt=0 after %d deleted)",
            (void*)res, hset_cnt(&se1), cnt_before
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 5. Непустое минус строгое подмножество */
    test_sub("subtest %d: non-empty minus subset", ++subnum);
    {
        int     all_vals[] = {1, 2, 3, 4, 5, 6};
        int     sub_vals[] = {2, 5};
        hset    se1 = hset_from_intarr(all_vals, COUNT(all_vals));
        hset    se2 = hset_from_intarr(sub_vals, COUNT(sub_vals));
        /* hset    se2 = hset_init(10, VALUE64_INT);
        for (int i = 0; i < COUNT(sub_vals); i++)
            hset_set(&se2, LITERAL64_INT(sub_vals[i])); */

        hset   *res = hset_minus(&se1, &se2);

        int     expected_cnt = 4;   // {1,3,4,6}
        int     ok = (res == &se1) && (hset_cnt(&se1) == expected_cnt) &&
                     !hset_get(&se1, LITERAL64_INT(2)) &&
                     !hset_get(&se1, LITERAL64_INT(5)) &&
                     hset_get(&se1, LITERAL64_INT(1)) &&
                     hset_get(&se1, LITERAL64_INT(3)) &&
                     hset_get(&se1, LITERAL64_INT(4)) &&
                     hset_get(&se1, LITERAL64_INT(6));
        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Non-empty minus subset: res=%p, cnt=%d (expected res=&se1, cnt=%d)",
            (void*)res, hset_cnt(&se1), expected_cnt
        );
        hset_free(&se1);
        hset_free(&se2);
    }
    /* 6. Непустое минус непересекающееся множество */
    test_sub("subtest %d: non-empty minus disjoint", ++subnum);
    {
        int     vals1[] = {7, 8, 9};
        int     vals2[] = {1, 2, 3};
        hset    se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset    se2 = hset_from_intarr(vals2, COUNT(vals2));

        int     cnt_before = hset_cnt(&se1);
        hset   *res = hset_minus(&se1, &se2);

        int     ok = (res == &se1) && (hset_cnt(&se1) == cnt_before);
        for (int i = 0; ok && i < COUNT(vals1); i++)
            ok = hset_get(&se1, LITERAL64_INT(vals1[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Non-empty minus disjoint: res=%p, cnt=%d (expected res=&se1, cnt=%d)",
            (void*)res, hset_cnt(&se1), cnt_before
        );
        hset_free(&se1);
        hset_free(&se2);
    }
    /* 7. Удаление, когда se2 содержит лишние элементы */
    test_sub("subtest %d: se2 with extra elements", ++subnum);
    {
        int     vals1[] = {100, 200, 300};
        int     vals2[] = {200, 400, 500};
        hset    se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset    se2 = hset_from_intarr(vals2, COUNT(vals2));

        hset   *res = hset_minus(&se1, &se2);

        // Ожидаем, что останутся только 100 и 300
        int     expected_remaining[] = {100, 300};
        int     ok = (res == &se1) && (hset_cnt(&se1) == COUNT(expected_remaining));
        for (int i = 0; ok && i < COUNT(expected_remaining); i++)
            ok = hset_get(&se1, LITERAL64_INT(expected_remaining[i]));
        // Убедимся, что 200 действительно удалён
        if (ok)
            ok = !hset_get(&se1, LITERAL64_INT(200));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "se2 with extra elements: res=%p, cnt=%d (expected res=&se1, cnt=2)",
            (void*)res, hset_cnt(&se1)
        );
        hset_free(&se1);
        hset_free(&se2);
    }
    /* 8. Несовпадение типов – ожидаем аварийный сигнал */
    test_sub("subtest %d: type mismatch raises SIGINT", ++subnum);
    {
        int int_vals[] = {1, 2, 3};
        hset int_set = hset_from_intarr(int_vals, COUNT(int_vals) );
        hset dbl_set = hset_init(10, VALUE64_DBL);
        hset_set(&dbl_set, LITERAL64_DBL(1.0));

        bool signal_caught = try();
        if (signal_caught) {
            hset_free(&int_set);
            hset_free(&dbl_set);
            return logret(TEST_PASSED, "Signal caught as expected");
        } else {
            hset_free(&int_set);
            hset_free(&dbl_set);
            return logerr(TEST_FAILED, "Expected signal for type mismatch, but none was raised");
        }
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 11 ---------------------------------
static TestStatus
tf11(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Пустое минус пустое = пустое */
    test_sub("subtest %d: empty minus empty", ++subnum);
    {
        hset se1 = hset_init(10, VALUE64_INT);
        hset se2 = hset_init(10, VALUE64_INT);
        hset res = hset_init_minus(&se1, &se2);

        int ok = (hset_cnt(&res) == 0);
        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2), hset_free(&res)),
            "Empty minus empty: cnt=%d (expected 0)", hset_cnt(&res)
        );
        hset_free(&se1);
        hset_free(&se2);
        hset_free(&res);
    }

    /* 2. Непустое минус пустое = копия исходного (все элементы) */
    test_sub("subtest %d: non-empty minus empty", ++subnum);
    {
        int vals[] = {3, 7, 11, 42};
        hset se1 = hset_from_intarr(vals, COUNT(vals));
        hset empty = hset_init(10, VALUE64_INT);
        hset res = hset_init_minus(&se1, &empty);

        int ok = (hset_cnt(&res) == COUNT(vals));
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&res, LITERAL64_INT(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&empty), hset_free(&res)),
            "Non-empty minus empty: cnt=%d (expected %d)", hset_cnt(&res), COUNT(vals)
        );
        hset_free(&se1);
        hset_free(&empty);
        hset_free(&res);
    }

    /* 3. Пустое минус непустое = пустое */
    test_sub("subtest %d: empty minus non-empty", ++subnum);
    {
        int vals[] = {100, 200};
        hset empty = hset_init(10, VALUE64_INT);
        hset se2 = hset_from_intarr(vals, COUNT(vals));
        hset res = hset_init_minus(&empty, &se2);

        int ok = (hset_cnt(&res) == 0);
        test_validatefree(
            ok,
            (hset_free(&empty), hset_free(&se2), hset_free(&res)),
            "Empty minus non-empty: cnt=%d (expected 0)", hset_cnt(&res)
        );
        hset_free(&empty);
        hset_free(&se2);
        hset_free(&res);
    }

    /* 4. Множество минус его копия = пустое (все элементы удаляются) */
    test_sub("subtest %d: set minus its copy", ++subnum);
    {
        int vals[] = {5, 10, 15, 20};
        hset se1 = hset_from_intarr(vals, COUNT(vals));
        hset se2 = hset_clone(&se1);  // или ручное заполнение, но clone уже проверен
        hset res = hset_init_minus(&se1, &se2);

        int ok = (hset_cnt(&res) == 0);
        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2), hset_free(&res)),
            "Set minus its copy: cnt=%d (expected 0)", hset_cnt(&res)
        );
        hset_free(&se1);
        hset_free(&se2);
        hset_free(&res);
    }

    /* 5. Частичное перекрытие: элементы, отсутствующие в se2, сохраняются */
    test_sub("subtest %d: partial overlap", ++subnum);
    {
        int vals1[] = {1, 2, 3, 4, 5, 6};
        int vals2[] = {2, 5};
        hset se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset se2 = hset_init(10, VALUE64_INT);
        for (int i = 0; i < COUNT(vals2); i++)
            hset_set(&se2, LITERAL64_INT(vals2[i]));

        hset res = hset_init_minus(&se1, &se2);

        // Ожидаем, что останутся {1,3,4,6}
        int expected_vals[] = {1, 3, 4, 6};
        int ok = (hset_cnt(&res) == COUNT(expected_vals));
        for (int i = 0; ok && i < COUNT(expected_vals); i++)
            ok = hset_get(&res, LITERAL64_INT(expected_vals[i]));

        // Дополнительно убедимся, что удалённых нет
        if (ok) {
            ok = !hset_get(&res, LITERAL64_INT(2)) &&
                 !hset_get(&res, LITERAL64_INT(5));
        }

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2), hset_free(&res)),
            "Partial overlap: cnt=%d (expected %d)", hset_cnt(&res), COUNT(expected_vals)
        );
        hset_free(&se1);
        hset_free(&se2);
        hset_free(&res);
    }

    /* 6. Непересекающиеся множества: все элементы se1 должны остаться */
    test_sub("subtest %d: disjoint sets", ++subnum);
    {
        int vals1[] = {7, 8, 9};
        int vals2[] = {1, 2, 3};
        hset se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset se2 = hset_from_intarr(vals2, COUNT(vals2));
        hset res = hset_init_minus(&se1, &se2);

        int ok = (hset_cnt(&res) == COUNT(vals1));
        for (int i = 0; ok && i < COUNT(vals1); i++)
            ok = hset_get(&res, LITERAL64_INT(vals1[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2), hset_free(&res)),
            "Disjoint sets: cnt=%d (expected %d)", hset_cnt(&res), COUNT(vals1)
        );
        hset_free(&se1);
        hset_free(&se2);
        hset_free(&res);
    }
    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 12 ---------------------------------

static TestStatus
tf12(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Пустое пересекается с пустым – остаётся пустое */
    test_sub("subtest %d: empty intersect empty", ++subnum);
    {
        hset    se1 = hset_init(10, VALUE64_INT);
        hset    se2 = hset_init(10, VALUE64_INT);

        hset   *res = hset_intersect(&se1, &se2);

        int     ok = (res == &se1) && (hset_cnt(&se1) == 0);
        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Empty intersect empty: res=%p, cnt=%d (expected res=&se1, cnt=0)",
            (void*)res, hset_cnt(&se1)
        );
        hset_free(&se1);
        hset_free(&se2);
    }
    /* 2. Непустое пересекается с пустым – остаётся пустое */
    test_sub("subtest %d: non-empty intersect empty", ++subnum);
    {
        int     vals[] = {1, 2, 3};
        hset    se1 = hset_from_intarr(vals, COUNT(vals));
        hset    empty = hset_init(10, VALUE64_INT);
        hset   *res = hset_intersect(&se1, &empty);

        int     ok = (res == &se1) && (hset_cnt(&se1) == 0) &&
                     (hset_cnt(&empty) == 0);
        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&empty)),
            "Non-empty intersect empty: res=%p, cnt=%d (expected res=&se1, cnt=0)",
            (void*)res, hset_cnt(&se1)
        );
        hset_free(&se1);
        hset_free(&empty);
    }
    /* 3. Пустое пересекается с непустым – остаётся пустое */
    test_sub("subtest %d: empty intersect non-empty", ++subnum);
    {
        int     vals[] = {10, 20};
        hset    empty = hset_init(10, VALUE64_INT);
        hset    se2 = hset_from_intarr(vals, COUNT(vals));
        hset   *res = hset_intersect(&empty, &se2);

        int     ok = (res == &empty) && (hset_cnt(&empty) == 0) &&
                     (hset_cnt(&se2) == COUNT(vals));
        test_validatefree(
            ok,
            (hset_free(&empty), hset_free(&se2)),
            "Empty intersect non-empty: res=%p, cnt=%d (expected res=&empty, cnt=0)",
            (void*)res, hset_cnt(&empty)
        );
        hset_free(&empty);
        hset_free(&se2);
    }
    /* 4. Одинаковые множества – остаются все элементы */
    test_sub("subtest %d: identical sets", ++subnum);
    {
        int     vals[] = {7, 8, 9};
        hset    se1 = hset_from_intarr(vals, COUNT(vals));
        hset    se2 = hset_from_intarr(vals, COUNT(vals));
        int     cnt_before = hset_cnt(&se1);

        hset   *res = hset_intersect(&se1, &se2);

        int     ok = (res == &se1) && (hset_cnt(&se1) == cnt_before);
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&se1, LITERAL64_INT(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Identical sets: res=%p, cnt=%d (expected res=&se1, cnt=%d)",
            (void*)res, hset_cnt(&se1), cnt_before
        );
        hset_free(&se1);
        hset_free(&se2);
    }
    /* 5. Частичное перекрытие – остаются только общие элементы */
    test_sub("subtest %d: partial overlap", ++subnum);
    {
        int     vals1[] = {1, 2, 3, 4, 5};
        int     vals2[] = {2, 4, 6};
        hset    se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset    se2 = hset_from_intarr(vals2, COUNT(vals2));

        hset   *res = hset_intersect(&se1, &se2);

        int     expected_vals[] = {2, 4};
        int     expected_cnt = COUNT(expected_vals);
        int     ok = (res == &se1) && (hset_cnt(&se1) == expected_cnt);
        for (int i = 0; ok && i < expected_cnt; i++)
            ok = hset_get(&se1, LITERAL64_INT(expected_vals[i]));
        // Убедимся, что остальные отсутствуют
        if (ok) {
            ok = !hset_get(&se1, LITERAL64_INT(1)) &&
                 !hset_get(&se1, LITERAL64_INT(3)) &&
                 !hset_get(&se1, LITERAL64_INT(5));
        }

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Partial overlap: res=%p, cnt=%d (expected res=&se1, cnt=%d)",
            (void*)res, hset_cnt(&se1), expected_cnt
        );
        hset_free(&se1);
        hset_free(&se2);
    }
    /* 6. Непересекающиеся множества – результат пуст */
    test_sub("subtest %d: disjoint sets", ++subnum);
    {
        int     vals1[] = {100, 200};
        int     vals2[] = {300, 400};
        hset    se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset    se2 = hset_from_intarr(vals2, COUNT(vals2));

        hset   *res = hset_intersect(&se1, &se2);

        int     ok = (res == &se1) && (hset_cnt(&se1) == 0) &&
                     (hset_cnt(&se2) == COUNT(vals2));
        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Disjoint sets: res=%p, cnt=%d (expected res=&se1, cnt=0)",
            (void*)res, hset_cnt(&se1)
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 13 ---------------------------------

static TestStatus
tf13(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Пустое с пустым -> пустое */
    test_sub("subtest %d: empty intersect empty", ++subnum);
    {
        hset se1 = hset_init(10, VALUE64_INT);
        hset se2 = hset_init(10, VALUE64_INT);
        hset res = hset_init_intersect(&se1, &se2);

        int ok = (hset_cnt(&res) == 0) && (hset_cnt(&se1) == 0) && (hset_cnt(&se2) == 0);
        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2), hset_free(&res)),
            "Empty intersect empty: res=%d, se1=%d, se2=%d (expected all 0)",
            hset_cnt(&res), hset_cnt(&se1), hset_cnt(&se2)
        );
        hset_free(&se1);
        hset_free(&se2);
        hset_free(&res);
    }

    /* 2. Непустое с пустым -> пустое, se1 не изменилось */
    test_sub("subtest %d: non-empty intersect empty", ++subnum);
    {
        int vals[] = {10, 20, 30};
        hset se1 = hset_from_intarr(vals, COUNT(vals));
        hset empty = hset_init(10, VALUE64_INT);
        hset res = hset_init_intersect(&se1, &empty);

        int ok = (hset_cnt(&res) == 0) && (hset_cnt(&se1) == COUNT(vals)) && (hset_cnt(&empty) == 0);
        // дополнительно убедимся, что элементы se1 на месте
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&se1, LITERAL64_INT(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&empty), hset_free(&res)),
            "Non-empty intersect empty: res=%d, se1=%d, empty=%d (expected res=0, se1=%d)",
            hset_cnt(&res), hset_cnt(&se1), hset_cnt(&empty), COUNT(vals)
        );
        hset_free(&se1);
        hset_free(&empty);
        hset_free(&res);
    }

    /* 3. Пустое с непустым -> пустое, se2 не изменилось */
    test_sub("subtest %d: empty intersect non-empty", ++subnum);
    {
        int vals[] = {5, 7};
        hset empty = hset_init(10, VALUE64_INT);
        hset se2 = hset_from_intarr(vals, COUNT(vals));
        hset res = hset_init_intersect(&empty, &se2);

        int ok = (hset_cnt(&res) == 0) && (hset_cnt(&empty) == 0) && (hset_cnt(&se2) == COUNT(vals));
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&se2, LITERAL64_INT(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&empty), hset_free(&se2), hset_free(&res)),
            "Empty intersect non-empty: res=%d, empty=%d, se2=%d (expected res=0, se2=%d)",
            hset_cnt(&res), hset_cnt(&empty), hset_cnt(&se2), COUNT(vals)
        );
        hset_free(&empty);
        hset_free(&se2);
        hset_free(&res);
    }

    /* 4. Одинаковые множества -> все элементы */
    test_sub("subtest %d: identical sets", ++subnum);
    {
        int vals[] = {1, 2, 3, 4};
        hset se1 = hset_from_intarr(vals, COUNT(vals));
        hset se2 = hset_from_intarr(vals, COUNT(vals));
        hset res = hset_init_intersect(&se1, &se2);

        int expected = COUNT(vals);
        int ok = (hset_cnt(&res) == expected) && (hset_cnt(&se1) == expected) && (hset_cnt(&se2) == expected);
        for (int i = 0; ok && i < expected; i++)
            ok = hset_get(&res, LITERAL64_INT(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2), hset_free(&res)),
            "Identical sets: res=%d, se1=%d, se2=%d (expected all %d)",
            hset_cnt(&res), hset_cnt(&se1), hset_cnt(&se2), expected
        );
        hset_free(&se1);
        hset_free(&se2);
        hset_free(&res);
    }

    /* 5. Частичное перекрытие -> только общие элементы */
    test_sub("subtest %d: partial overlap", ++subnum);
    {
        int vals1[] = {1, 2, 3, 4, 5};
        int vals2[] = {2, 4, 6};
        hset se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset se2 = hset_from_intarr(vals2, COUNT(vals2));
        hset res = hset_init_intersect(&se1, &se2);

        int common_vals[] = {2, 4};
        int expected = COUNT(common_vals);
        int ok = (hset_cnt(&res) == expected) && (hset_cnt(&se1) == COUNT(vals1)) && (hset_cnt(&se2) == COUNT(vals2));
        for (int i = 0; ok && i < expected; i++)
            ok = hset_get(&res, LITERAL64_INT(common_vals[i]));
        // убедимся, что не общих элементов нет
        if (ok) {
            ok = !hset_get(&res, LITERAL64_INT(1)) &&
                 !hset_get(&res, LITERAL64_INT(3)) &&
                 !hset_get(&res, LITERAL64_INT(5));
        }

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2), hset_free(&res)),
            "Partial overlap: res=%d, se1=%d, se2=%d (expected res=%d)",
            hset_cnt(&res), hset_cnt(&se1), hset_cnt(&se2), expected
        );
        hset_free(&se1);
        hset_free(&se2);
        hset_free(&res);
    }

    /* 6. Непересекающиеся множества -> пустое */
    test_sub("subtest %d: disjoint sets", ++subnum);
    {
        int vals1[] = {100, 200};
        int vals2[] = {300, 400};
        hset se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset se2 = hset_from_intarr(vals2, COUNT(vals2));
        hset res = hset_init_intersect(&se1, &se2);

        int ok = (hset_cnt(&res) == 0) && (hset_cnt(&se1) == COUNT(vals1)) && (hset_cnt(&se2) == COUNT(vals2));
        // se1 и se2 не должны потерять элементы
        for (int i = 0; ok && i < COUNT(vals1); i++)
            ok = hset_get(&se1, LITERAL64_INT(vals1[i]));
        for (int i = 0; ok && i < COUNT(vals2); i++)
            ok = hset_get(&se2, LITERAL64_INT(vals2[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2), hset_free(&res)),
            "Disjoint sets: res=%d, se1=%d, se2=%d (expected res=0)",
            hset_cnt(&res), hset_cnt(&se1), hset_cnt(&se2)
        );
        hset_free(&se1);
        hset_free(&se2);
        hset_free(&res);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 14 ---------------------------------
static TestStatus
tf14(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Пустое с пустым -> пустое */
    test_sub("subtest %d: empty symm diff empty", ++subnum);
    {
        hset se1 = hset_init(10, VALUE64_INT);
        hset se2 = hset_init(10, VALUE64_INT);
        hset res = hset_init_symmdiff(&se1, &se2);

        int ok = (hset_cnt(&res) == 0) && (hset_cnt(&se1) == 0) && (hset_cnt(&se2) == 0);
        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2), hset_free(&res)),
            "Empty symm diff empty: res=%d, se1=%d, se2=%d (expected all 0)",
            hset_cnt(&res), hset_cnt(&se1), hset_cnt(&se2)
        );
        hset_free(&se1);
        hset_free(&se2);
        hset_free(&res);
    }

    /* 2. Непустое с пустым -> все элементы непустого */
    test_sub("subtest %d: non-empty symm diff empty", ++subnum);
    {
        int vals[] = {3, 7, 11};
        hset se1 = hset_from_intarr(vals, COUNT(vals));
        hset empty = hset_init(10, VALUE64_INT);
        hset res = hset_init_symmdiff(&se1, &empty);

        int expected = COUNT(vals);
        int ok = (hset_cnt(&res) == expected) && (hset_cnt(&se1) == expected) && (hset_cnt(&empty) == 0);
        for (int i = 0; ok && i < expected; i++)
            ok = hset_get(&res, LITERAL64_INT(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&empty), hset_free(&res)),
            "Non-empty symm diff empty: res=%d, se1=%d, empty=%d (expected res=%d)",
            hset_cnt(&res), hset_cnt(&se1), hset_cnt(&empty), expected
        );
        hset_free(&se1);
        hset_free(&empty);
        hset_free(&res);
    }

    /* 3. Пустое с непустым -> все элементы непустого */
    test_sub("subtest %d: empty symm diff non-empty", ++subnum);
    {
        int vals[] = {5, 9};
        hset empty = hset_init(10, VALUE64_INT);
        hset se2 = hset_from_intarr(vals, COUNT(vals));
        hset res = hset_init_symmdiff(&empty, &se2);

        int expected = COUNT(vals);
        int ok = (hset_cnt(&res) == expected) && (hset_cnt(&empty) == 0) && (hset_cnt(&se2) == expected);
        for (int i = 0; ok && i < expected; i++)
            ok = hset_get(&res, LITERAL64_INT(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&empty), hset_free(&se2), hset_free(&res)),
            "Empty symm diff non-empty: res=%d, empty=%d, se2=%d (expected res=%d)",
            hset_cnt(&res), hset_cnt(&empty), hset_cnt(&se2), expected
        );
        hset_free(&empty);
        hset_free(&se2);
        hset_free(&res);
    }

    /* 4. Одинаковые множества -> пустое */
    test_sub("subtest %d: identical sets", ++subnum);
    {
        int vals[] = {1, 2, 3, 4};
        hset se1 = hset_from_intarr(vals, COUNT(vals));
        hset se2 = hset_from_intarr(vals, COUNT(vals));
        hset res = hset_init_symmdiff(&se1, &se2);

        int expected = COUNT(vals);
        int ok = (hset_cnt(&res) == 0) && (hset_cnt(&se1) == expected) && (hset_cnt(&se2) == expected);
        // элементы исходных множеств на месте
        for (int i = 0; ok && i < expected; i++) {
            if (!hset_get(&se1, LITERAL64_INT(vals[i])) ||
                !hset_get(&se2, LITERAL64_INT(vals[i])))
                ok = 0;
        }

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2), hset_free(&res)),
            "Identical sets: res=%d, se1=%d, se2=%d (expected res=0)",
            hset_cnt(&res), hset_cnt(&se1), hset_cnt(&se2)
        );
        hset_free(&se1);
        hset_free(&se2);
        hset_free(&res);
    }

    /* 5. Частичное перекрытие -> только элементы, принадлежащие ровно одному множеству */
    test_sub("subtest %d: partial overlap", ++subnum);
    {
        int vals1[] = {1, 2, 3, 4, 5};
        int vals2[] = {2, 4, 6};
        hset se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset se2 = hset_from_intarr(vals2, COUNT(vals2));
        hset res = hset_init_symmdiff(&se1, &se2);

        // Ожидаем {1,3,5,6}
        int expected_vals[] = {1, 3, 5, 6};
        int expected_cnt = COUNT(expected_vals);
        int ok = (hset_cnt(&res) == expected_cnt) &&
                 (hset_cnt(&se1) == COUNT(vals1)) &&
                 (hset_cnt(&se2) == COUNT(vals2));
        // Проверяем наличие ожидаемых
        for (int i = 0; ok && i < expected_cnt; i++)
            ok = hset_get(&res, LITERAL64_INT(expected_vals[i]));
        // Проверяем отсутствие общих (2 и 4)
        if (ok) {
            ok = !hset_get(&res, LITERAL64_INT(2)) &&
                 !hset_get(&res, LITERAL64_INT(4));
        }
        // Убедимся, что исходные множества не изменились
        for (int i = 0; ok && i < COUNT(vals1); i++)
            ok = hset_get(&se1, LITERAL64_INT(vals1[i]));
        for (int i = 0; ok && i < COUNT(vals2); i++)
            ok = hset_get(&se2, LITERAL64_INT(vals2[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2), hset_free(&res)),
            "Partial overlap: res=%d, se1=%d, se2=%d (expected res=%d)",
            hset_cnt(&res), hset_cnt(&se1), hset_cnt(&se2), expected_cnt
        );
        hset_free(&se1);
        hset_free(&se2);
        hset_free(&res);
    }

    /* 6. Непересекающиеся множества -> все элементы обоих */
    test_sub("subtest %d: disjoint sets", ++subnum);
    {
        int vals1[] = {100, 200};
        int vals2[] = {300, 400};
        hset se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset se2 = hset_from_intarr(vals2, COUNT(vals2));
        hset res = hset_init_symmdiff(&se1, &se2);

        int expected_total = COUNT(vals1) + COUNT(vals2);
        int ok = (hset_cnt(&res) == expected_total) &&
                 (hset_cnt(&se1) == COUNT(vals1)) &&
                 (hset_cnt(&se2) == COUNT(vals2));
        // все элементы vals1 и vals2 должны быть в res
        for (int i = 0; ok && i < COUNT(vals1); i++)
            ok = hset_get(&res, LITERAL64_INT(vals1[i]));
        for (int i = 0; ok && i < COUNT(vals2); i++)
            ok = hset_get(&res, LITERAL64_INT(vals2[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2), hset_free(&res)),
            "Disjoint sets: res=%d, se1=%d, se2=%d (expected res=%d)",
            hset_cnt(&res), hset_cnt(&se1), hset_cnt(&se2), expected_total
        );
        hset_free(&se1);
        hset_free(&se2);
        hset_free(&res);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 15 ---------------------------------
static TestStatus
tf15(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Пустое с пустым -> пустое */
    test_sub("subtest %d: empty symmdiff empty", ++subnum);
    {
        hset se1 = hset_init(10, VALUE64_INT);
        hset se2 = hset_init(10, VALUE64_INT);
        hset *res = hset_symmdiff(&se1, &se2);

        int ok = (hset_cnt(&se1) == 0) && (hset_cnt(&se2) == 0) && (res == &se1);
        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Empty symmdiff empty: se1=%d, se2=%d, res=%p (expected se1=0, se2=0, res==&se1)",
            hset_cnt(&se1), hset_cnt(&se2), (void*)res
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 2. Непустое с пустым -> se1 без изменений */
    test_sub("subtest %d: non-empty symmdiff empty", ++subnum);
    {
        int vals[] = {3, 7, 11};
        hset se1 = hset_from_intarr(vals, COUNT(vals));
        hset empty = hset_init(10, VALUE64_INT);
        int cnt_before = hset_cnt(&se1);
        hset *res = hset_symmdiff(&se1, &empty);

        int ok = (hset_cnt(&se1) == cnt_before) && (hset_cnt(&empty) == 0) && (res == &se1);
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&se1, LITERAL64_INT(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&empty)),
            "Non-empty symmdiff empty: se1=%d, empty=%d, res=%p (expected se1=%d, empty=0)",
            hset_cnt(&se1), hset_cnt(&empty), (void*)res, cnt_before
        );
        hset_free(&se1);
        hset_free(&empty);
    }

    /* 3. Пустое с непустым -> se1 получает все элементы se2 */
    test_sub("subtest %d: empty symmdiff non-empty", ++subnum);
    {
        int vals[] = {5, 9};
        hset se1 = hset_init(10, VALUE64_INT);
        hset se2 = hset_from_intarr(vals, COUNT(vals));
        hset *res = hset_symmdiff(&se1, &se2);

        int expected = COUNT(vals);
        int ok = (hset_cnt(&se1) == expected) && (hset_cnt(&se2) == expected) && (res == &se1);
        for (int i = 0; ok && i < expected; i++)
            ok = hset_get(&se1, LITERAL64_INT(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Empty symmdiff non-empty: se1=%d, se2=%d, res=%p (expected se1=%d, se2=%d)",
            hset_cnt(&se1), hset_cnt(&se2), (void*)res, expected, expected
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 4. Одинаковые множества -> se1 становится пустым */
    test_sub("subtest %d: identical sets", ++subnum);
    {
        int vals[] = {1, 2, 3, 4};
        hset se1 = hset_from_intarr(vals, COUNT(vals));
        hset se2 = hset_from_intarr(vals, COUNT(vals));
        hset *res = hset_symmdiff(&se1, &se2);

        int expected_orig = COUNT(vals);
        int ok = (hset_cnt(&se1) == 0) && (hset_cnt(&se2) == expected_orig) && (res == &se1);
        // элементы se2 должны остаться на месте
        for (int i = 0; ok && i < expected_orig; i++)
            ok = hset_get(&se2, LITERAL64_INT(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Identical sets: se1=%d, se2=%d, res=%p (expected se1=0, se2=%d)",
            hset_cnt(&se1), hset_cnt(&se2), (void*)res, expected_orig
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 5. Частичное перекрытие -> se1 содержит только элементы, принадлежащие ровно одному множеству */
    test_sub("subtest %d: partial overlap", ++subnum);
    {
        int vals1[] = {1, 2, 3, 4, 5};
        int vals2[] = {2, 4, 6};
        hset se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset se2 = hset_from_intarr(vals2, COUNT(vals2));
        hset *res = hset_symmdiff(&se1, &se2);

        // Ожидаем в se1: {1,3,5,6}
        int expected_vals[] = {1, 3, 5, 6};
        int expected_cnt = COUNT(expected_vals);
        int ok = (hset_cnt(&se1) == expected_cnt) && (hset_cnt(&se2) == COUNT(vals2)) && (res == &se1);
        // проверяем наличие ожидаемых
        for (int i = 0; ok && i < expected_cnt; i++)
            ok = hset_get(&se1, LITERAL64_INT(expected_vals[i]));
        // проверяем отсутствие общих (2 и 4)
        if (ok) {
            ok = !hset_get(&se1, LITERAL64_INT(2)) &&
                 !hset_get(&se1, LITERAL64_INT(4));
        }
        // se2 не изменился
        for (int i = 0; ok && i < COUNT(vals2); i++)
            ok = hset_get(&se2, LITERAL64_INT(vals2[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Partial overlap: se1=%d, se2=%d, res=%p (expected se1=%d, se2=%d)",
            hset_cnt(&se1), hset_cnt(&se2), (void*)res, expected_cnt, COUNT(vals2)
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 6. Непересекающиеся множества -> se1 содержит все элементы обоих */
    test_sub("subtest %d: disjoint sets", ++subnum);
    {
        int vals1[] = {100, 200};
        int vals2[] = {300, 400};
        hset se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset se2 = hset_from_intarr(vals2, COUNT(vals2));
        hset *res = hset_symmdiff(&se1, &se2);

        int expected_total = COUNT(vals1) + COUNT(vals2);
        int ok = (hset_cnt(&se1) == expected_total) && (hset_cnt(&se2) == COUNT(vals2)) && (res == &se1);
        // все элементы vals1 и vals2 должны быть в se1
        for (int i = 0; ok && i < COUNT(vals1); i++)
            ok = hset_get(&se1, LITERAL64_INT(vals1[i]));
        for (int i = 0; ok && i < COUNT(vals2); i++)
            ok = hset_get(&se1, LITERAL64_INT(vals2[i]));
        // se2 не изменился
        for (int i = 0; ok && i < COUNT(vals2); i++)
            ok = hset_get(&se2, LITERAL64_INT(vals2[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Disjoint sets: se1=%d, se2=%d, res=%p (expected se1=%d, se2=%d)",

            hset_cnt(&se1), hset_cnt(&se2), (void*)res, expected_total, COUNT(vals2)
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 16 ---------------------------------
static TestStatus
tf16(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Пустое множество увеличиваем – остаётся пустым */
    test_sub("subtest %d: empty resize larger", ++subnum);
    {
        hset se = hset_init(10, VALUE64_INT);          // sz станет 11
        int old_sz = se.sz;
        hset new_se = hset_init_resize(&se, 30);    // запросим 30

        int ok = (se.sz > old_sz) && (se.sz >= 30) &&
                 (hset_cnt(&se) == 0) && (new_se.sz == se.sz);
        test_validatefree(
            ok,
            hset_free(&se),
            "Empty resize: old_sz=%d -> new_sz=%d, cnt=%d (expected cnt=0)",
            old_sz, se.sz, hset_cnt(&se)
        );
        test_validatefree(
            hset_validate(logfile, &se), hset_free(&se),
            "Validation failed"
        );
        hset_free(&se);
    }

    /* 2. Непустое множество увеличиваем – все элементы сохраняются */
    test_sub("subtest %d: non‑empty resize larger", ++subnum);
    {
        int vals[] = {1, 2, 3, 4, 5};
        hset se = hset_from_intarr(vals, COUNT(vals));
        int old_cnt = hset_cnt(&se);
        int old_sz = se.sz;
        hset_init_resize(&se, 100);

        int ok = (se.sz > old_sz) && (hset_cnt(&se) == old_cnt);
        // проверяем, что все значения доступны
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&se, LITERAL64_INT(vals[i]));

        test_validatefree(
            ok,
            hset_free(&se),
            "Non‑empty resize larger: old_sz=%d -> new_sz=%d, cnt=%d (expected %d)",
            old_sz, se.sz, hset_cnt(&se), old_cnt
        );
        test_validatefree(
            hset_validate(logfile, &se), hset_free(&se),
            "Validation failed"
        );
        hset_free(&se);
    }

    /* 3. Непустое множество уменьшаем (но не меньше количества элементов) */
    test_sub("subtest %d: non‑empty resize smaller", ++subnum);
    {
        int vals[] = {10, 20, 30};
        hset se = hset_from_intarr(vals, COUNT(vals));
        int old_cnt = hset_cnt(&se);
        int old_sz = se.sz;
        hset_init_resize(&se, 3);   // 3 элемента, next_prime(3) = 3

        // Размер может стать меньше, но элементы должны остаться
        int ok = (hset_cnt(&se) == old_cnt);
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&se, LITERAL64_INT(vals[i]));

        test_validatefree(
            ok,
            hset_free(&se),
            "Non‑empty resize smaller: old_sz=%d -> new_sz=%d, cnt=%d (expected %d)",
            old_sz, se.sz, hset_cnt(&se), old_cnt
        );
        test_validatefree(
            hset_validate(logfile, &se), hset_free(&se),
            "Validation failed"
        );
        hset_free(&se);
    }

    /* 4. Попытка переразмерить на тот же размер – ничего не меняется */
    test_sub("subtest %d: resize to same size", ++subnum);
    {
        int vals[] = {7, 8, 9};
        hset se = hset_from_intarr(vals, COUNT(vals));
        int old_sz = se.sz;
        int old_cnt = hset_cnt(&se);
        hset_init_resize(&se, old_sz - 1); // передаём sz-1, чтобы next_prime вернуло old_sz

        int ok = (se.sz == old_sz) && (hset_cnt(&se) == old_cnt);
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&se, LITERAL64_INT(vals[i]));

        test_validatefree(
            ok,
            hset_free(&se),
            "Resize to same size: sz should stay %d, got %d, cnt=%d",
            old_sz, se.sz, hset_cnt(&se)
        );
        test_validatefree(
            hset_validate(logfile, &se), hset_free(&se),
            "Validation failed"
        );
        hset_free(&se);
    }

    /* 5. Последовательные ресайзы */
    test_sub("subtest %d: multiple resizes", ++subnum);
    {
        int vals[] = {100, 200, 300, 400};
        hset se = hset_from_intarr(vals, COUNT(vals));
        int cnt = hset_cnt(&se);

        hset_init_resize(&se, 50);
        hset_init_resize(&se, 10);
        hset_init_resize(&se, 200);

        int ok = (hset_cnt(&se) == cnt);
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&se, LITERAL64_INT(vals[i]));

        test_validatefree(
            ok,
            hset_free(&se),
            "Multiple resizes: final sz=%d, cnt=%d (expected %d)",
            se.sz, hset_cnt(&se), cnt
        );
        test_validatefree(
            hset_validate(logfile, &se), hset_free(&se),
            "Validation failed"
        );
        hset_free(&se);
    }

    /* 6. Ресайз с типом double (проверка, что тип не теряется) */
    test_sub("subtest %d: resize with double", ++subnum);
    {
        double dvals[] = {1.5, 2.5, 3.5};
        hset se = hset_from_dblarr(dvals, COUNT(dvals));
        int old_cnt = hset_cnt(&se);
        hset_init_resize(&se, 50);

        int ok = (hset_cnt(&se) == old_cnt) && (se.flags == VALUE64_DBL);
        for (int i = 0; ok && i < COUNT(dvals); i++)
            ok = hset_get(&se, LITERAL64_DBL(dvals[i]));

        test_validatefree(
            ok,
            hset_free(&se),
            "Resize double: sz=%d, cnt=%d (expected %d)",
            se.sz, hset_cnt(&se), old_cnt
        );
        test_validatefree(
            hset_validate(logfile, &se), hset_free(&se),
            "Validation failed"
        );
        hset_free(&se);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 17 ---------------------------------
static TestStatus
tf17(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Пустое ∪ пустое = пустое */
    test_sub("subtest %d: empty union empty", ++subnum);
    {
        hset    se1 = hset_init(10, VALUE64_INT);
        hset    se2 = hset_init(10, VALUE64_INT);
        hset   *res = hset_union(&se1, &se2);

        int     ok = (hset_cnt(&se1) == 0) && (res == &se1);
        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Empty union empty: cnt=%d, res=%p (expected cnt=0, res=&se1)",
            hset_cnt(&se1), (void*)res
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 2. Непустое ∪ пустое = исходное непустое */
    test_sub("subtest %d: non-empty union empty", ++subnum);
    {
        int     vals[] = {3, 7, 11};
        hset    se1 = hset_from_intarr(vals, COUNT(vals));
        hset    empty = hset_init(10, VALUE64_INT);
        int     cnt_before = hset_cnt(&se1);
        hset   *res = hset_union(&se1, &empty);

        int     ok = (hset_cnt(&se1) == cnt_before) && (res == &se1);
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&se1, LITERAL64_INT(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&empty)),
            "Non-empty union empty: cnt=%d (expected %d), res=%p",
            hset_cnt(&se1), cnt_before, (void*)res
        );
        hset_free(&se1);
        hset_free(&empty);
    }

    /* 3. Пустое ∪ непустое = копия непустого в se1 */
    test_sub("subtest %d: empty union non-empty", ++subnum);
    {
        int     vals[] = {5, 9};
        hset    se1 = hset_init(10, VALUE64_INT);
        hset    se2 = hset_from_intarr(vals, COUNT(vals));
        hset   *res = hset_union(&se1, &se2);

        int     expected = COUNT(vals);
        int     ok = (hset_cnt(&se1) == expected) && (res == &se1);
        for (int i = 0; ok && i < expected; i++)
            ok = hset_get(&se1, LITERAL64_INT(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Empty union non-empty: cnt=%d (expected %d), res=%p",
            hset_cnt(&se1), expected, (void*)res
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 4. Одинаковые множества: результат не меняется, количество не растёт */
    test_sub("subtest %d: identical sets", ++subnum);
    {
        int     vals[] = {1, 2, 3, 4};
        hset    se1 = hset_from_intarr(vals, COUNT(vals));
        hset    se2 = hset_from_intarr(vals, COUNT(vals));
        int     cnt_before = hset_cnt(&se1);
        hset   *res = hset_union(&se1, &se2);

        int     ok = (hset_cnt(&se1) == cnt_before) && (res == &se1);
        // все элементы должны остаться на месте
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&se1, LITERAL64_INT(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Identical sets: cnt=%d (expected %d), res=%p",
            hset_cnt(&se1), cnt_before, (void*)res
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 5. Частичное перекрытие: добавляются только недостающие элементы */
    test_sub("subtest %d: partial overlap", ++subnum);
    {
        int     vals1[] = {1, 2, 3, 4, 5};
        int     vals2[] = {3, 5, 6, 7};
        hset    se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset    se2 = hset_from_intarr(vals2, COUNT(vals2));
        hset   *res = hset_union(&se1, &se2);

        // Ожидаем {1,2,3,4,5,6,7} – 7 элементов
        int     expected_vals[] = {1, 2, 3, 4, 5, 6, 7};
        int     expected_cnt = COUNT(expected_vals);
        int     ok = (hset_cnt(&se1) == expected_cnt) && (res == &se1);
        for (int i = 0; ok && i < expected_cnt; i++)
            ok = hset_get(&se1, LITERAL64_INT(expected_vals[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Partial overlap: cnt=%d (expected %d), res=%p",
            hset_cnt(&se1), expected_cnt, (void*)res
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 6. Непересекающиеся множества: все элементы se2 добавляются */
    test_sub("subtest %d: disjoint sets", ++subnum);
    {
        int     vals1[] = {100, 200};
        int     vals2[] = {300, 400};
        hset    se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset    se2 = hset_from_intarr(vals2, COUNT(vals2));
        hset   *res = hset_union(&se1, &se2);

        int     expected_total = COUNT(vals1) + COUNT(vals2);
        int     ok = (hset_cnt(&se1) == expected_total) && (res == &se1);
        // все элементы должны быть доступны
        for (int i = 0; ok && i < COUNT(vals1); i++)
            ok = hset_get(&se1, LITERAL64_INT(vals1[i]));
        for (int i = 0; ok && i < COUNT(vals2); i++)
            ok = hset_get(&se1, LITERAL64_INT(vals2[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2)),
            "Disjoint sets: cnt=%d (expected %d), res=%p",
            hset_cnt(&se1), expected_total, (void*)res
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 18 ---------------------------------

static TestStatus
tf18(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Пустое ∪ пустое = пустое */
    test_sub("subtest %d: empty union empty", ++subnum);
    {
        hset    se1 = hset_init(10, VALUE64_INT);
        hset    se2 = hset_init(10, VALUE64_INT);
        hset    res = hset_init_union(&se1, &se2);

        int     ok = (hset_cnt(&res) == 0) &&
                     (hset_cnt(&se1) == 0) &&
                     (hset_cnt(&se2) == 0);
        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2), hset_free(&res)),
            "Empty union empty: res=%d, se1=%d, se2=%d (expected all 0)",
            hset_cnt(&res), hset_cnt(&se1), hset_cnt(&se2)
        );
        hset_free(&se1);
        hset_free(&se2);
        hset_free(&res);
    }

    /* 2. Непустое ∪ пустое = копия se1, исходные не изменились */
    test_sub("subtest %d: non-empty union empty", ++subnum);
    {
        int     vals[] = {3, 7, 11};
        hset    se1 = hset_from_intarr(vals, COUNT(vals));
        hset    empty = hset_init(10, VALUE64_INT);

        int     cnt1_before = hset_cnt(&se1);
        hset    res = hset_init_union(&se1, &empty);
        int     expected = COUNT(vals);

        int     ok = (hset_cnt(&res) == expected) &&
                     (hset_cnt(&se1) == cnt1_before) &&
                     (hset_cnt(&empty) == 0);
        for (int i = 0; ok && i < expected; i++)
            ok = hset_get(&res, LITERAL64_INT(vals[i]));
        // se1 должен остаться без изменений
        for (int i = 0; ok && i < expected; i++)
            ok = hset_get(&se1, LITERAL64_INT(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&empty), hset_free(&res)),
            "Non-empty union empty: res=%d, se1=%d (expected %d, %d)",
            hset_cnt(&res), hset_cnt(&se1), expected, cnt1_before
        );
        hset_free(&se1);
        hset_free(&empty);
        hset_free(&res);
    }

    /* 3. Пустое ∪ непустое = копия se2, исходные не изменились */
    test_sub("subtest %d: empty union non-empty", ++subnum);
    {
        int     vals[] = {5, 9};
        hset    empty = hset_init(10, VALUE64_INT);
        hset    se2 = hset_from_intarr(vals, COUNT(vals));

        int     cnt2_before = hset_cnt(&se2);
        hset    res = hset_init_union(&empty, &se2);

        int     expected = COUNT(vals);
        int     ok = (hset_cnt(&res) == expected) &&
                     (hset_cnt(&empty) == 0) &&
                     (hset_cnt(&se2) == cnt2_before);
        for (int i = 0; ok && i < expected; i++)
            ok = hset_get(&res, LITERAL64_INT(vals[i]));
        for (int i = 0; ok && i < expected; i++)
            ok = hset_get(&se2, LITERAL64_INT(vals[i]));

        test_validatefree(
            ok,
            (hset_free(&empty), hset_free(&se2), hset_free(&res)),
            "Empty union non-empty: res=%d, se2=%d (expected %d, %d)",
            hset_cnt(&res), hset_cnt(&se2), expected, cnt2_before
        );
        hset_free(&empty);
        hset_free(&se2);
        hset_free(&res);
    }

    /* 4. Одинаковые множества: результат совпадает с исходными */
    test_sub("subtest %d: identical sets", ++subnum);
    {
        int     vals[] = {1, 2, 3, 4};
        hset    se1 = hset_from_intarr(vals, COUNT(vals));
        hset    se2 = hset_from_intarr(vals, COUNT(vals));

        int     cnt_before = hset_cnt(&se1);
        hset    res = hset_init_union(&se1, &se2);

        int     expected = COUNT(vals);
        int     ok = (hset_cnt(&res) == expected) &&
                     (hset_cnt(&se1) == cnt_before) &&
                     (hset_cnt(&se2) == cnt_before);
        for (int i = 0; ok && i < expected; i++) {
            ok = hset_get(&res, LITERAL64_INT(vals[i])) &&
                 hset_get(&se1, LITERAL64_INT(vals[i])) &&
                 hset_get(&se2, LITERAL64_INT(vals[i]));
        }

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2), hset_free(&res)),
            "Identical sets: res=%d, se1=%d, se2=%d (expected all %d)",
            hset_cnt(&res), hset_cnt(&se1), hset_cnt(&se2), expected
        );
        hset_free(&se1);
        hset_free(&se2);
        hset_free(&res);
    }

    /* 5. Частичное перекрытие: объединение содержит все уникальные элементы */
    test_sub("subtest %d: partial overlap", ++subnum);
    {
        int     vals1[] = {1, 2, 3, 4, 5};
        int     vals2[] = {3, 5, 6, 7};
        hset    se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset    se2 = hset_from_intarr(vals2, COUNT(vals2));

        int     cnt1_before = hset_cnt(&se1);
        int     cnt2_before = hset_cnt(&se2);
        hset    res = hset_init_union(&se1, &se2);

        int     expected_vals[] = {1, 2, 3, 4, 5, 6, 7};
        int     expected_cnt = COUNT(expected_vals);
        int     ok = (hset_cnt(&res) == expected_cnt) &&
                     (hset_cnt(&se1) == cnt1_before) &&
                     (hset_cnt(&se2) == cnt2_before);
        for (int i = 0; ok && i < expected_cnt; i++)
            ok = hset_get(&res, LITERAL64_INT(expected_vals[i]));
        // se1 и se2 не тронуты
        for (int i = 0; ok && i < COUNT(vals1); i++)
            ok = hset_get(&se1, LITERAL64_INT(vals1[i]));
        for (int i = 0; ok && i < COUNT(vals2); i++)
            ok = hset_get(&se2, LITERAL64_INT(vals2[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2), hset_free(&res)),
            "Partial overlap: res=%d, se1=%d, se2=%d (expected res=%d)",
            hset_cnt(&res), hset_cnt(&se1), hset_cnt(&se2), expected_cnt
        );
        hset_free(&se1);
        hset_free(&se2);
        hset_free(&res);
    }

    /* 6. Непересекающиеся множества: все элементы обоих в результате */
    test_sub("subtest %d: disjoint sets", ++subnum);
    {
        int     vals1[] = {100, 200};
        int     vals2[] = {300, 400};
        hset    se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset    se2 = hset_from_intarr(vals2, COUNT(vals2));

        int     cnt1_before = hset_cnt(&se1);
        int     cnt2_before = hset_cnt(&se2);
        hset    res = hset_init_union(&se1, &se2);

        int     expected_total = COUNT(vals1) + COUNT(vals2);
        int     ok = (hset_cnt(&res) == expected_total) &&
                     (hset_cnt(&se1) == cnt1_before) &&
                     (hset_cnt(&se2) == cnt2_before);
        for (int i = 0; ok && i < COUNT(vals1); i++)
            ok = hset_get(&res, LITERAL64_INT(vals1[i]));
        for (int i = 0; ok && i < COUNT(vals2); i++)
            ok = hset_get(&res, LITERAL64_INT(vals2[i]));
        // исходные целы
        for (int i = 0; ok && i < COUNT(vals1); i++)
            ok = hset_get(&se1, LITERAL64_INT(vals1[i]));
        for (int i = 0; ok && i < COUNT(vals2); i++)
            ok = hset_get(&se2, LITERAL64_INT(vals2[i]));

        test_validatefree(
            ok,
            (hset_free(&se1), hset_free(&se2), hset_free(&res)),
            "Disjoint sets: res=%d, se1=%d, se2=%d (expected res=%d)",
            hset_cnt(&res), hset_cnt(&se1), hset_cnt(&se2), expected_total
        );
        hset_free(&se1);
        hset_free(&se2);
        hset_free(&res);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 19 ---------------------------------

static TestStatus
tf19(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Пустое множество: load factor = 0, должен уменьшиться до HSET_MIN_SIZE */
    test_sub("subtest %d: empty set normalized", ++subnum);
    {
        hset    se = hset_init(100, VALUE64_INT);    // sz будет 101
        int     old_sz = se.sz;
        int     old_cnt = hset_cnt(&se);
        hset    res = hset_normalize(&se);

        int     ok = (res.sz == se.sz) &&                     // возврат совпадает с se
                     (hset_cnt(&se) == old_cnt) &&           // пусто
                     (se.sz <= old_sz) &&                    // размер уменьшился
                     (se.sz >= HSET_MIN_SIZE);               // но не меньше минимума
        test_validatefree(
            ok,
            hset_free(&se),
            "Empty set: old_sz=%d -> new_sz=%d, cnt=%d (expected sz <= %d, >= %d)",
            old_sz, se.sz, hset_cnt(&se), old_sz, HSET_MIN_SIZE
        );
        hset_free(&se);
    }

    /* 2. Низкая загрузка (много пустых корзин) -> размер уменьшается */
    test_sub("subtest %d: low load factor", ++subnum);
    {
        int     vals[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        hset    se = hset_init(1000, VALUE64_INT);   // сразу задаём большой размер
        hset_loadiarr(&se, vals, COUNT(vals));    // загружаем элементы

        int     old_sz = se.sz;
        int     cnt_before = hset_cnt(&se);
        hset    res = hset_normalize(&se);

        int     ok = (res.sz == se.sz) && (hset_cnt(&se) == cnt_before) &&
                     (se.sz < old_sz);            // размер должен уменьшиться
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&se, LITERAL64_INT(vals[i]));

        test_validatefree(
            ok,
            hset_free(&se),
            "Low load factor: old_sz=%d -> new_sz=%d, cnt=%d (expected sz < %d)",
            old_sz, se.sz, hset_cnt(&se), old_sz
        );
        hset_free(&se);
    }
    /* 3. Высокая загрузка (>0.75) -> размер увеличивается */
    test_sub("subtest %d: high load factor", ++subnum);
    {
        int     vals[100];
        for (int i = 0; i < COUNT(vals); i++)
            vals[i] = i;

        hset    se = hset_init(100, VALUE64_INT);   // next_prime(100) = 101
        hset_loadiarr(&se, vals, COUNT(vals));   // 100 элементов, load factor ≈ 0.99
        int     old_sz = se.sz;
        int     cnt_before = hset_cnt(&se);
        hset    res = hset_normalize(&se);

        int     ok = (res.sz == se.sz) && (hset_cnt(&se) == cnt_before) &&
                     (se.sz > old_sz);            // размер должен вырасти
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&se, LITERAL64_INT(vals[i]));

        test_validatefree(
            ok,
            hset_free(&se),
            "High load factor: old_sz=%d -> new_sz=%d, cnt=%d (expected sz > %d)",
            old_sz, se.sz, hset_cnt(&se), old_sz
        );
        hset_free(&se);
    }
    /* 4. Нормальная загрузка (в пределах 0.25..0.75) -> размер не меняется */
    test_sub("subtest %d: normal load factor unchanged", ++subnum);
    {
        int     vals[20];
        for (int i = 0; i < COUNT(vals); i++)
            vals[i] = i;

        hset    se = hset_init(50, VALUE64_INT);           // sz станет 53 (next_prime(50))
        hset_loadiarr(&se, vals, COUNT(vals));          // 20 элементов, load factor ≈ 0.38
        int     old_sz = se.sz;
        int     cnt_before = hset_cnt(&se);
        hset    res = hset_normalize(&se);

        int     ok = (res.sz == se.sz) && (hset_cnt(&se) == cnt_before) &&
                     (se.sz == old_sz);                 // размер не должен измениться
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&se, LITERAL64_INT(vals[i]));

        test_validatefree(
            ok,
            hset_free(&se),
            "Normal load factor: old_sz=%d -> new_sz=%d, cnt=%d (expected sz unchanged %d)",
            old_sz, se.sz, hset_cnt(&se), old_sz
        );
        hset_free(&se);
    }
    /* 5. После нормализации все элементы доступны (уже проверялось, но оставим явный) */
    test_sub("subtest %d: elements preserved after normalize", ++subnum);
    {
        int     vals[] = {5, 15, 25, 35, 45};
        hset    se = hset_from_intarr(vals, COUNT(vals));
        hset_normalize(&se);   // после создания load factor ~5/7=0.71 – может не измениться, но проверим

        int     ok = (hset_cnt(&se) == COUNT(vals));
        for (int i = 0; ok && i < COUNT(vals); i++)
            ok = hset_get(&se, LITERAL64_INT(vals[i]));

        test_validatefree(
            ok,
            hset_free(&se),
            "Elements preserved: cnt=%d (expected %d)",
            hset_cnt(&se), COUNT(vals)
        );
        hset_free(&se);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 20 ---------------------------------

static TestStatus
tf20(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Сохранить и загрузить пустое множество */
    test_sub("subtest %d: empty save/load", ++subnum);
    {
        hset    se = hset_init(10, VALUE64_INT);

        int     save_ret = hset_save("res/hashset/empty.hset", &se);

        test_validatefree(
            save_ret >= 0,
            hset_free(&se),
            "Empty save failed, ret=%d", save_ret
        );

        hset    loaded = HSET_NONINIT;
        save_ret = hset_load("res/hashset/empty.hset", &loaded);

        test_validatefree(
            save_ret >= 0,
            hset_free(&se),
            "Empty save failed, ret=%d", save_ret
        );
        test_validatefree(
            hset_cnt(&loaded) == 0,
            (hset_free(&se), hset_free(&loaded)),
            "Empty set: cnt=%d, expected 0", hset_cnt(&loaded)
        );
        test_validatefree(
            hset_validate(logfile, &loaded),
            (hset_free(&se), hset_free(&loaded)),
            "Empty set: validation failed"
        );
        test_validatefree(
            hset_eq(&se, &loaded),
            (hset_free(&se), hset_free(&loaded)),
            "Empty set: not equal after load"
        );

        hset_free(&se);
        hset_free(&loaded);
    }

    /* 2. Сохранить и загрузить непустое множество (int) — создание нового */
    test_sub("subtest %d: int save/load new", ++subnum);
    {
        int     vals[] = {19, 21, 13, 4555, 5678};
        hset    se = hset_from_intarr(vals, COUNT(vals));
        int     save_ret = hset_save("res/hashset/int.hset", &se);

        test_validatefree(
            save_ret == COUNT(vals),
            hset_free(&se),
            "Int save returned %d, expected %d", save_ret, COUNT(vals)
        );

        hset    loaded = HSET_NONINIT;
        int     load_ret = hset_load("res/hashset/int.hset", &loaded);

        test_validatefree(
            load_ret == COUNT(vals),
            (hset_free(&se), hset_free(&loaded)),
            "Int load returned %d, expected %d", load_ret, COUNT(vals)
        );
        test_validatefree(
            hset_cnt(&loaded) == COUNT(vals),
            (hset_free(&se), hset_free(&loaded)),
            "Int set: cnt=%d, expected %d", hset_cnt(&loaded), COUNT(vals)
        );
        test_validatefree(
            hset_validate(logfile, &loaded),
            (hset_free(&se), hset_free(&loaded)),
            "Int set: validation failed"
        );
        test_validatefree(
            hset_eq(&se, &loaded),
            (hset_free(&se), hset_free(&loaded)),
            "Int set: not equal after load"
        );

        hset_free(&se);
        hset_free(&loaded);
    }

    /* 3. Дозагрузка в существующее множество (int) */
    test_sub("subtest %d: int append load", ++subnum);
    {
        int     vals1[] = {10, 20};
        int     vals2[] = {30, 40};
        hset    se = hset_from_intarr(vals1, COUNT(vals1));

        hset    tmp = hset_from_intarr(vals2, COUNT(vals2));
        int     save_ret = hset_save("res/hashset/append_int.hset", &tmp);
        hset_free(&tmp);

        test_validatefree(
            save_ret == COUNT(vals2),
            hset_free(&se),
            "Append save returned %d, expected %d", save_ret, COUNT(vals2)
        );

        int     load_ret = hset_load("res/hashset/append_int.hset", &se);
        test_validatefree(
            load_ret == COUNT(vals2),
            hset_free(&se),
            "Append load returned %d, expected %d", load_ret, COUNT(vals2)
        );

        int     all_vals[] = {10, 20, 30, 40};
        hset    expected = hset_from_intarr(all_vals, COUNT(all_vals));

        test_validatefree(
            hset_cnt(&se) == COUNT(all_vals),
            (hset_free(&se), hset_free(&expected)),
            "Append: cnt=%d, expected %d", hset_cnt(&se), COUNT(all_vals)
        );
        test_validatefree(
            hset_validate(logfile, &se),
            (hset_free(&se), hset_free(&expected)),
            "Append: validation failed"
        );
        test_validatefree(
            hset_eq(&se, &expected),
            (hset_free(&se), hset_free(&expected)),
            "Append: not equal to expected"
        );

        hset_free(&se);
        hset_free(&expected);
    }

    /* 4. Несовпадение типов — возвращается -1, множество не меняется */
    test_sub("subtest %d: type mismatch returns -1", ++subnum);
    {
        hset    se_int = hset_init(10, VALUE64_INT);
        hset_save("res/hashset/type_int.hset", &se_int);

        hset    se_dbl = hset_init(10, VALUE64_DBL);
        int     ret = hset_load("res/hashset/type_int.hset", &se_dbl);

        test_validatefree(
            ret == -1,
            (hset_free(&se_int), hset_free(&se_dbl)),
            "Type mismatch: return code %d, expected -1", ret
        );
        test_validatefree(
            hset_cnt(&se_dbl) == 0,
            (hset_free(&se_int), hset_free(&se_dbl)),
            "Type mismatch: target set cnt=%d, expected 0 (unchanged)", hset_cnt(&se_dbl)
        );

        hset_free(&se_int);
        hset_free(&se_dbl);
    }

    /* 5. Сохранить и загрузить long множество */
    test_sub("subtest %d: long save/load new", ++subnum);
    {
        long    vals[] = {100L, 200L, 300L, 400L, 500L};
        hset    se = hset_from_longarr(vals, COUNT(vals));
        int     save_ret = hset_save("res/hashset/long.hset", &se);

        test_validatefree(
            save_ret == COUNT(vals),
            hset_free(&se),
            "Append save returned %d, expected %d", save_ret, COUNT(vals)
        );

        hset    loaded = HSET_NONINIT;
        int     load_ret = hset_load("res/hashset/long.hset", &loaded);

        test_validatefree(
            load_ret == COUNT(vals),
            hset_free(&se),
            "Append load returned %d, expected %d", load_ret, COUNT(vals)
        );

        test_validatefree(
            hset_cnt(&loaded) == COUNT(vals),
            (hset_free(&se), hset_free(&loaded)),
            "Long set: cnt=%d, expected %d", hset_cnt(&loaded), COUNT(vals)
        );
        test_validatefree(
            hset_validate(logfile, &loaded),
            (hset_free(&se), hset_free(&loaded)),
            "Long set: validation failed"
        );
        test_validatefree(
            hset_eq(&se, &loaded),
            (hset_free(&se), hset_free(&loaded)),
            "Long set: not equal after load"
        );

        hset_free(&se);
        hset_free(&loaded);
    }

    /* 6. Сохранить и загрузить double множество */
    test_sub("subtest %d: double save/load new", ++subnum);
    {
        double  vals[] = {1.1, 2.2, 3.3, 4.4, 5.5};
        hset    se = hset_from_dblarr(vals, COUNT(vals));
        int     save_ret = hset_save("res/hashset/double.hset", &se);

        test_validatefree(
            save_ret == COUNT(vals),
            hset_free(&se),
            "Append save returned %d, expected %d", save_ret, COUNT(vals)
        );

        hset    loaded = HSET_NONINIT;
        int     load_ret = hset_load("res/hashset/double.hset", &loaded);

        test_validatefree(
            load_ret == COUNT(vals),
            hset_free(&se),
            "Append load returned %d, expected %d", load_ret, COUNT(vals)
        );
        test_validatefree(
            hset_cnt(&loaded) == COUNT(vals),
            (hset_free(&se), hset_free(&loaded)),
            "Double set: cnt=%d, expected %d", hset_cnt(&loaded), COUNT(vals)
        );
        test_validatefree(
            hset_validate(logfile, &loaded),
            (hset_free(&se), hset_free(&loaded)),
            "Double set: validation failed"
        );
        test_validatefree(
            hset_eq(&se, &loaded),
            (hset_free(&se), hset_free(&loaded)),
            "Double set: not equal after load"
        );

        hset_free(&se);
        hset_free(&loaded);
    }
    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 21 ---------------------------------

static void             print_as_int(value64 v){
    printf("%4d\t", v.ival);
}

static void             dummy_const_proc(value64 v) {
    (void)v; /* ничего не делаем */
}
/*
static void             multiply_by_two(value64 *v) {
    v->ival *= 2;
}

static void             remove_if_even(hset *se, hset_elem *el) {
    if (el->v.ival % 2 == 0)
        hset_del(se, el->v);
}

static void             remove_all(hset *se, hset_elem *el) {
    hset_del(se, el->v);
} */

static TestStatus
tf21(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ---------- 1. hset_const_foreach: только чтение ---------- */
    test_sub("subtest %d: const_foreach read-only", ++subnum);
    {
        int     vals[] = {10, 20, 30, 40, 50};
        hset    se = hset_from_intarr(vals, COUNT(vals) );
        // just a printf
        hset_const_foreach(&se, print_as_int);
        hset_free(&se);
    }

    // Так как мы убрали ctx, для сбора данных в const_foreach нужен другой подход.
    // Пока оставим простую проверку: просто вызовем и убедимся, что множество не изменилось.
    test_sub("subtest %d: const_foreach does not modify", ++subnum);
    {
        int     vals[] = {1, 2, 3};
        hset    se = hset_from_intarr(vals, COUNT(vals));

        hset_const_foreach(&se, dummy_const_proc); // ничего не делает, но гарантирует проход

        // Проверяем, что все элементы на месте и их количество не изменилось
        test_validatefree(
            hset_cnt(&se) == COUNT(vals),
            hset_free(&se),
            "const_foreach: cnt=%d, expected %d", hset_cnt(&se), COUNT(vals)
        );
        for (int i = 0; i < COUNT(vals); i++) {
            test_validatefree(
                hset_get(&se, LITERAL64_INT(vals[i])),
                hset_free(&se),
                "const_foreach: missing value %d", vals[i]
            );
        }
        hset_free(&se);
    }

    return logret(TEST_PASSED, "done");
}
// ------------------------- TEST 22 ---------------------------------
static TestStatus
tf22(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Сумма int */
    test_sub("subtest %d: reduce sum int", ++subnum);
    {
        int     vals[] = {1, 2, 3, 4, 5};
        hset    se = hset_from_intarr(vals, COUNT(vals));
        hset_accum res = hset_reduce(&se, hset_sum_int);

        test_validatefree(
            res.value.ival == 15,
            hset_free(&se),
            "Sum int: value = %d, expected 15", res.value.ival
        );
        test_validatefree(
            res.count == 5,
            hset_free(&se),
            "Sum int: count = %d, expected 5", res.count
        );
        hset_free(&se);
    }

    /* 2. Счётчик int */
    test_sub("subtest %d: reduce count int", ++subnum);
    {
        int     vals[] = {10, 20, 30};
        hset    se = hset_from_intarr(vals, COUNT(vals));
        hset_accum res = hset_reduce(&se, hset_count_int);

        test_validatefree(
            res.count == COUNT(vals),
            hset_free(&se),
            "Count int: count = %d, expected %d", res.count, COUNT(vals)
        );
        // value не должно измениться (осталось нулевым)
        test_validatefree(
            res.value.ival == 0,
            hset_free(&se),
            "Count int: value = %d, expected 0 (unused)", res.value.ival
        );
        hset_free(&se);
    }

    /* 3. Максимум int */
    test_sub("subtest %d: reduce max int", ++subnum);
    {
        int     vals[] = {5, 2, 9, 1, 7};
        hset    se = hset_from_intarr(vals, COUNT(vals));
        hset_accum res = hset_reduce(&se, hset_max_int);

        test_validatefree(
            res.value.ival == 9,
            hset_free(&se),
            "Max int: value = %d, expected 9", res.value.ival
        );
        test_validatefree(
            res.count == 1,
            hset_free(&se),
            "Max int: count = %d, expected 1 (flag)", res.count
        );
        hset_free(&se);
    }

    /* 4. Минимум int */
    test_sub("subtest %d: reduce min int", ++subnum);
    {
        int     vals[] = {3, 8, 1, 6, 4};
        hset    se = hset_from_intarr(vals, COUNT(vals));
        hset_accum res = hset_reduce(&se, hset_min_int);

        test_validatefree(
            res.value.ival == 1,
            hset_free(&se),
            "Min int: value = %d, expected 1", res.value.ival
        );
        test_validatefree(
            res.count == 1,
            hset_free(&se),
            "Min int: count = %d, expected 1 (flag)", res.count
        );
        hset_free(&se);
    }

    /* 5. Пустое множество: сумма и счётчик */
    test_sub("subtest %d: reduce on empty set", ++subnum);
    {
        hset    se = hset_init(10, VALUE64_INT);
        hset_accum res = hset_reduce(&se, hset_sum_int);

        test_validatefree(
            res.value.ival == 0,
            hset_free(&se),
            "Empty sum: value = %d, expected 0", res.value.ival
        );
        test_validatefree(
            res.count == 0,
            hset_free(&se),
            "Empty sum: count = %d, expected 0", res.count
        );
        hset_free(&se);
    }

    /* 6. Пустое множество: max (count остаётся 0) */
    test_sub("subtest %d: reduce max on empty set", ++subnum);
    {
        hset    se = hset_init(10, VALUE64_INT);
        hset_accum res = hset_reduce(&se, hset_max_int);

        test_validatefree(
            res.count == 0,
            hset_free(&se),
            "Empty max: count = %d, expected 0 (no elements)", res.count
        );
        // value не важно, т.к. нет элементов
        hset_free(&se);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 23 ---------------------------------

static TestStatus
tf23(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Сумма double (только конечные числа) */
    test_sub("subtest %d: sum double", ++subnum);
    {
        double  vals[] = {1.5, 2.5, 3.5};
        hset    se = hset_from_dblarr(vals, COUNT(vals));
        hset_accum res = hset_initreduce(&se, HSET_ACCUM_DBL_ZERO, hset_sum_dbl);

        test_validatefree(
            fabs(res.value.dval - 7.5) < 0.0001,
            hset_free(&se),
            "Sum double: value = %f, expected 7.5", res.value.dval
        );
        test_validatefree(
            res.count == 3,
            hset_free(&se),
            "Sum double: count = %d, expected 3", res.count
        );
        hset_free(&se);
    }

    /* 2. Счётчик double (только конечные числа) */
    test_sub("subtest %d: count double", ++subnum);
    {
        double  vals[] = {10.0, 20.0, 30.0};
        hset    se = hset_from_dblarr(vals, COUNT(vals));
        hset_accum res = hset_initreduce(&se, HSET_ACCUM_DBL_ZERO, hset_count_dbl);

        test_validatefree(
            res.count == 3,
            hset_free(&se),
            "Count double: count = %d, expected 3", res.count
        );
        test_validatefree(
            res.value.dval == 0.0,
            hset_free(&se),
            "Count double: value = %f, expected 0.0 (unused)", res.value.dval
        );
        hset_free(&se);
    }

    /* 3. Максимум double */
    test_sub("subtest %d: max double", ++subnum);
    {
        double  vals[] = {5.2, 2.3, 9.8, 1.0, 7.4};
        hset    se = hset_from_dblarr(vals, COUNT(vals));
        hset_accum res = hset_initreduce(&se, HSET_ACCUM_DBL_ZERO, hset_max_dbl);

        test_validatefree(
            fabs(res.value.dval - 9.8) < 0.0001,
            hset_free(&se),
            "Max double: value = %f, expected 9.8", res.value.dval
        );
        test_validatefree(
            res.count == 1,
            hset_free(&se),
            "Max double: count = %d, expected 1 (flag)", res.count
        );
        hset_free(&se);
    }

    /* 4. Минимум double (все отрицательные) */
    test_sub("subtest %d: min double (all negative)", ++subnum);
    {
        double  vals[] = {-5.0, -2.5, -9.1, -1.0};
        hset    se = hset_from_dblarr(vals, COUNT(vals));
        hset_accum res = hset_initreduce(&se, HSET_ACCUM_DBL_ZERO, hset_min_dbl);

        test_validatefree(
            fabs(res.value.dval - (-9.1)) < 0.0001,
            hset_free(&se),
            "Min double: value = %f, expected -9.1", res.value.dval
        );
        test_validatefree(
            res.count == 1,
            hset_free(&se),
            "Min double: count = %d, expected 1 (flag)", res.count
        );
        hset_free(&se);
    }

    /* 5. Пустое множество: сумма double */
    test_sub("subtest %d: sum double on empty set", ++subnum);
    {
        hset    se = hset_init(10, VALUE64_DBL);
        hset_accum res = hset_initreduce(&se, HSET_ACCUM_DBL_ZERO, hset_sum_dbl);

        test_validatefree(
            res.value.dval == 0.0,
            hset_free(&se),
            "Empty sum double: value = %f, expected 0.0", res.value.dval
        );
        test_validatefree(
            res.count == 0,
            hset_free(&se),
            "Empty sum double: count = %d, expected 0", res.count
        );
        hset_free(&se);
    }

    /* 6. Пустое множество: max double (count должен остаться 0) */
    test_sub("subtest %d: max double on empty set", ++subnum);
    {
        hset    se = hset_init(10, VALUE64_DBL);
        hset_accum res = hset_initreduce(&se, HSET_ACCUM_DBL_ZERO, hset_max_dbl);

        test_validatefree(
            res.count == 0,
            hset_free(&se),
            "Empty max double: count = %d, expected 0", res.count
        );
        hset_free(&se);
    }

    /* 7. Несколько значений, включая очень большое, для sum */
    test_sub("subtest %d: sum double with large value", ++subnum);
    {
        double  vals[] = {1e10, 2e10, 3e10};
        hset    se = hset_from_dblarr(vals, COUNT(vals));
        hset_accum res = hset_initreduce(&se, HSET_ACCUM_DBL_ZERO, hset_sum_dbl);

        test_validatefree(
            fabs(res.value.dval - 6e10) < 1e5,
            hset_free(&se),
            "Sum double large: value = %f, expected 6e10", res.value.dval
        );
        test_validatefree(
            res.count == 3,
            hset_free(&se),
            "Sum double large: count = %d, expected 3", res.count
        );
        hset_free(&se);
    }

    return logret(TEST_PASSED, "done");
}
// ------------------------- TEST 24 ---------------------------------
static TestStatus
tf24(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Уникальность NaN, +inf, -inf и корректность поиска */
    test_sub("subtest %d: double special values uniqueness", ++subnum);
    {
        hset    se = hset_init(10, VALUE64_DBL);

        double  normal_vals[] = {1.0, 2.0, 3.0};
        for (int i = 0; i < COUNT(normal_vals); i++)
            hset_set(&se, LITERAL64_DBL(normal_vals[i]));

        int     cnt_normal = hset_cnt(&se);
        test_validatefree(
            cnt_normal == COUNT(normal_vals),
            hset_free(&se),
            "After normal values: count = %d, expected %d", cnt_normal, COUNT(normal_vals)
        );

        /* ---- NaN ---- */
        test_validatefree(
            hset_set(&se, LITERAL64_DBL(NAN)),
            hset_free(&se),
            "First NaN was not added"
        );
        test_validatefree(
            hset_cnt(&se) == cnt_normal + 1,
            hset_free(&se),
            "After first NaN: count = %d, expected %d", hset_cnt(&se), cnt_normal + 1
        );

        test_validatefree(
            !hset_set(&se, LITERAL64_DBL(NAN)),
            hset_free(&se),
            "Second NaN was added, but should be duplicate"
        );
        test_validatefree(
            hset_cnt(&se) == cnt_normal + 1,
            hset_free(&se),
            "After second NaN attempt: count = %d, expected %d", hset_cnt(&se), cnt_normal + 1
        );

        /* ---- +inf ---- */
        test_validatefree(
            hset_set(&se, LITERAL64_DBL(INFINITY)),
            hset_free(&se),
            "First +inf was not added"
        );
        test_validatefree(
            hset_cnt(&se) == cnt_normal + 2,
            hset_free(&se),
            "After first +inf: count = %d, expected %d", hset_cnt(&se), cnt_normal + 2
        );

        test_validatefree(
            !hset_set(&se, LITERAL64_DBL(INFINITY)),
            hset_free(&se),
            "Second +inf was added, but should be duplicate"
        );
        test_validatefree(
            hset_cnt(&se) == cnt_normal + 2,
            hset_free(&se),
            "After second +inf attempt: count = %d, expected %d", hset_cnt(&se), cnt_normal + 2
        );

        /* ---- -inf ---- */
        test_validatefree(
            hset_set(&se, LITERAL64_DBL(-INFINITY)),
            hset_free(&se),
            "First -inf was not added"
        );
        test_validatefree(
            hset_cnt(&se) == cnt_normal + 3,
            hset_free(&se),
            "After first -inf: count = %d, expected %d", hset_cnt(&se), cnt_normal + 3
        );

        test_validatefree(
            !hset_set(&se, LITERAL64_DBL(-INFINITY)),
            hset_free(&se),
            "Second -inf was added, but should be duplicate"
        );
        test_validatefree(
            hset_cnt(&se) == cnt_normal + 3,
            hset_free(&se),
            "After second -inf attempt: count = %d, expected %d", hset_cnt(&se), cnt_normal + 3
        );

        /* ---- поиск всех значений ---- */
        for (int i = 0; i < COUNT(normal_vals); i++) {
            test_validatefree(
                hset_get(&se, LITERAL64_DBL(normal_vals[i])),
                hset_free(&se),
                "Missing normal value %f", normal_vals[i]
            );
        }
        test_validatefree(
            hset_get(&se, LITERAL64_DBL(NAN)),
            hset_free(&se),
            "Missing NaN after insertions"
        );
        test_validatefree(
            hset_get(&se, LITERAL64_DBL(INFINITY)),
            hset_free(&se),
            "Missing +inf after insertions"
        );
        test_validatefree(
            hset_get(&se, LITERAL64_DBL(-INFINITY)),
            hset_free(&se),
            "Missing -inf after insertions"
        );

        for (int i = 0; i < 15; i++)
            hset_set(&se, LITERAL64_DBL(i) );
        hset_tech_printall(se);

        /* ---- структурная целостность ---- */
        test_validatefree(
            hset_validate(logfile, &se),
            hset_free(&se),
            "Validation failed after special values"
        );

        hset_free(&se);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 25 ---------------------------------

static TestStatus
tf25(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Сумма int через макрос */
    test_sub("subtest %d: macro sum int", ++subnum);
    {
        int     vals[] = {1, 2, 3, 4, 5, 6, 7, 8};
        hset    se = hset_from_intarr(vals, COUNT(vals));
        //hset_techprint(&se, 0);
        int     sum = 0;
        HSET_FOREACH_INT(&se, v){
            //printf("%d\n", v);
            sum += v;
        }

        test_validatefree(
            sum == 36,
            hset_free(&se),
            "Macro sum int: sum = %d, expected 15", sum
        );
        hset_free(&se);
    }

    /* 2. Сбор элементов int в массив (проверяем, что все на месте) */
    test_sub("subtest %d: macro collect int", ++subnum);
    {
        int     vals[] = {3, 1, 4, 1, 5, 9, 2, 6};
        hset    se = hset_from_intarr(vals, COUNT(vals)); // дубликаты будут удалены
        int     expected[] = {1, 2, 3, 4, 5, 6, 9};   // уникальные отсортированные
        int     collected[COUNT(expected)];
        int     idx = 0;

        HSET_FOREACH_INT(&se, v) {
            if (idx < COUNT(expected))
                collected[idx++] = v;
        }

        test_validatefree(
            idx == COUNT(expected),
            hset_free(&se),
            "Macro collect int: collected %d elements, expected %d", idx, COUNT(expected)
        );
        // Сортируем, так как порядок обхода не гарантирован
        qsort(collected, idx, sizeof(int), pint_cmp);
        for (int i = 0; i < idx; i++) {
            test_validatefree(
                collected[i] == expected[i],
                hset_free(&se),
                "Macro collect int: collected[%d] = %d, expected %d", i, collected[i], expected[i]
            );
        }
        hset_free(&se);
    }

    /* 3. Проверка break: прерываем цикл при нахождении значения */
    test_sub("subtest %d: macro break int", ++subnum);
    {
        int     vals[] = {10, 20, 30, 40, 50};
        hset    se = hset_from_intarr(vals, COUNT(vals));
        int     found = 0;
        HSET_FOREACH_INT(&se, v) {
            if (v == 30) {
                found = 1;
                break;
            }
        }

        test_validatefree(
            found == 1,
            hset_free(&se),
            "Macro break int: value 30 %s found", found ? "" : "not"
        );
        hset_free(&se);
    }

    /* 4. Пустое множество: тело цикла не должно выполняться */
    test_sub("subtest %d: macro on empty set", ++subnum);
    {
        hset    se = hset_init(10, VALUE64_INT);
        int     called = 0;
        HSET_FOREACH_INT(&se, v){
            called++;
            (void) v;
        }

        test_validatefree(
            called == 0,
            hset_free(&se),
            "Macro on empty set: body executed %d times, expected 0", called
        );
        hset_free(&se);
    }

    /* 5. Double сумма через макрос */
    test_sub("subtest %d: macro sum double", ++subnum);
    {
        double  vals[] = {1.1, 2.2, 3.3, 4.4, 5.5};
        hset    se = hset_from_dblarr(vals, COUNT(vals));
        double  sum = 0.0;
        HSET_FOREACH_DBL(&se, v)
            sum += v;

        test_validatefree(
            fabs(sum - 16.5) < 0.0001,
            hset_free(&se),
            "Macro sum double: sum = %f, expected 16.5", sum
        );
        hset_free(&se);
    }
    /* 6. Double break: прерываем цикл при нахождении значения */
    test_sub("subtest %d: macro break double", ++subnum);
    {
        double  vals[] = {1.1, 2.2, 3.3, 4.4, 5.5};
        hset    se = hset_from_dblarr(vals, COUNT(vals));
        int     found = 0;
        HSET_FOREACH_DBL(&se, v) {
            if (fabs(v - 3.3) < 0.0001) {
                found = 1;
                break;
            }
        }

        test_validatefree(
            found == 1,
            hset_free(&se),
            "Macro break double: value 3.3 %s found", found ? "" : "not"
        );
        hset_free(&se);
    }

    /* 7. Long сумма и break */
    test_sub("subtest %d: macro sum long", ++subnum);
    {
        long    vals[] = {100L, 200L, 300L, 400L, 500L};
        hset    se = hset_from_longarr(vals, COUNT(vals));
        long    sum = 0;
        HSET_FOREACH_LONG(&se, v) sum += v;

        test_validatefree(
            sum == 1500L,
            hset_free(&se),
            "Macro sum long: sum = %ld, expected 1500", sum
        );
        hset_free(&se);
    }

    /* 8. Long break */
    test_sub("subtest %d: macro break long", ++subnum);
    {
        long    vals[] = {10L, 20L, 30L, 40L, 50L};
        hset    se = hset_from_longarr(vals, COUNT(vals));
        int     found = 0;
        HSET_FOREACH_LONG(&se, v) {
            if (v == 30L) {
                found = 1;
                break;
            }
        }

        test_validatefree(
            found == 1,
            hset_free(&se),
            "Macro break long: value 30 %s found", found ? "" : "not"
        );
        hset_free(&se);
    }

    /* 9. Указатели: проверяем, что компилируется и не падает (содержимое не проверяем) */
    test_sub("subtest %d: macro foreach pointer", ++subnum);
    {
        // Создадим множество указателей с тремя элементами
        int     a = 1, b = 2, c = 3;
        void   *vals[] = {&a, &b, &c};
        hset    se = hset_from_ptrarr( (const void **) vals, COUNT(vals));
        logauto(se.count);
        int     count = 0;
        HSET_FOREACH_PTR(&se, v) {
            count++;
            // просто убедимся, что v не NULL
            test_validatefree(
                v != NULL,
                hset_free(&se),
                "Macro foreach pointer: NULL pointer at pos %d", count
            );
        }

        test_validatefree(
            count == COUNT(vals),
            hset_free(&se),
            "Macro foreach pointer: count = %d, expected %d", count, COUNT(vals)
        );
        hset_free(&se);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 26 ---------------------------------
static TestStatus
tf26(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* ========== hset_any ========== */

    /* 1. any: пустое с пустым -> false (нет элементов для поиска) */
    test_sub("subtest %d:hset_any  empty vs empty", ++subnum);
    {
        hset se1 = hset_init(10, VALUE64_INT);
        hset se2 = hset_init(10, VALUE64_INT);
        test_validatefree(
            !hset_any(&se1, &se2),   // false
            (hset_free(&se1), hset_free(&se2)),
            "hset_any empty vs empty must be false"
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 2. any: пустое с непустым -> false */
    test_sub("subtest %d:hset_any  empty vs non-empty", ++subnum);
    {
        int vals[] = {1, 2, 3};
        hset empty = hset_init(10, VALUE64_INT);
        hset nonempty = hset_from_intarr(vals, COUNT(vals));
        test_validatefree(
            !hset_any(&empty, &nonempty),   // false
            (hset_free(&empty), hset_free(&nonempty)),
            "hset_any empty vs non-empty must be false"
        );
        hset_free(&empty);
        hset_free(&nonempty);
    }
    /* 3. any: непустое с пустым -> false (нет элементов в se2 для поиска) */
    test_sub("subtest %d: any non‑empty vs empty", ++subnum);
    {
        int vals[] = {1, 2, 3};
        hset empty = hset_init(10, VALUE64_INT);
        hset nonempty = hset_from_intarr(vals, COUNT(vals));
        test_validatefree(
            !hset_any(&nonempty, &empty),   // false
            (hset_free(&empty), hset_free(&nonempty)),
            "hset_any non‑empty vs empty must be false"
        );
        hset_free(&empty);
        hset_free(&nonempty);
    }
    /* 4. any: непустое с непустым, есть пересечение -> true */
    test_sub("subtest %d:hset_any  with overlap", ++subnum);
    {
        int vals1[] = {1, 3, 5, 7};
        int vals2[] = {5, 9, 11};
        hset se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset se2 = hset_from_intarr(vals2, COUNT(vals2));
        test_validatefree(
            hset_any(&se1, &se2),   // true
            (hset_free(&se1), hset_free(&se2)),
            "hset_any with overlap must be true"
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 5. any: непустое с непустым, нет пересечения -> false */
    test_sub("subtest %d:hset_any  disjoint", ++subnum);
    {
        int vals1[] = {100, 200};
        int vals2[] = {300, 400};
        hset se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset se2 = hset_from_intarr(vals2, COUNT(vals2));
        test_validatefree(
            !hset_any(&se1, &se2),   // false
            (hset_free(&se1), hset_free(&se2)),
            "hset_any disjoint must be false"
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 6. any: одинаковые множества -> true (все элементы se2 есть в se1) */
    test_sub("subtest %d:hset_any  identical sets", ++subnum);
    {
        int vals[] = {7, 8, 9};
        hset se1 = hset_from_intarr(vals, COUNT(vals));
        hset se2 = hset_from_intarr(vals, COUNT(vals));
        test_validatefree(
            hset_any(&se1, &se2),   // true
            (hset_free(&se1), hset_free(&se2)),
            "hset_any identical sets must be true"
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* ========== hset_notexists ========== */

    /* 7. notexists: пустое с пустым -> true (нет общих элементов) */
    test_sub("subtest %d: notexists empty vs empty", ++subnum);
    {
        hset se1 = hset_init(10, VALUE64_INT);
        hset se2 = hset_init(10, VALUE64_INT);
        test_validatefree(
            hset_notexists(&se1, &se2),   // true
            (hset_free(&se1), hset_free(&se2)),
            "hset_notexists empty vs empty must be true"
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 8. notexists: пустое с непустым -> true */
    test_sub("subtest %d: notexists empty vs non-empty", ++subnum);
    {
        int vals[] = {1, 2, 3};
        hset empty = hset_init(10, VALUE64_INT);
        hset nonempty = hset_from_intarr(vals, COUNT(vals));
        test_validatefree(
            hset_notexists(&empty, &nonempty),   // true
            (hset_free(&empty), hset_free(&nonempty)),
            "hset_notexists empty vs non-empty must be true"
        );
        hset_free(&empty);
        hset_free(&nonempty);
    }

    /* 9. notexists: непустое с непустым, нет пересечения -> true */
    test_sub("subtest %d: notexists disjoint", ++subnum);
    {
        int vals1[] = {10, 20};
        int vals2[] = {30, 40};
        hset se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset se2 = hset_from_intarr(vals2, COUNT(vals2));
        test_validatefree(
            hset_notexists(&se1, &se2),   // true
            (hset_free(&se1), hset_free(&se2)),
            "hset_notexists disjoint must be true"
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 10. notexists: непустое с непустым, есть пересечение -> false */
    test_sub("subtest %d: notexists with overlap", ++subnum);
    {
        int vals1[] = {1, 2, 3};
        int vals2[] = {3, 4, 5};
        hset se1 = hset_from_intarr(vals1, COUNT(vals1));
        hset se2 = hset_from_intarr(vals2, COUNT(vals2));
        test_validatefree(
            !hset_notexists(&se1, &se2),   // false
            (hset_free(&se1), hset_free(&se2)),
            "hset_notexists with overlap must be false"
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    /* 11. notexists: одинаковые множества -> false */
    test_sub("subtest %d: notexists identical sets", ++subnum);
    {
        int vals[] = {5, 6, 7};
        hset se1 = hset_from_intarr(vals, COUNT(vals));
        hset se2 = hset_from_intarr(vals, COUNT(vals));
        test_validatefree(
            !hset_notexists(&se1, &se2),   // false
            (hset_free(&se1), hset_free(&se2)),
            "hset_notexists identical sets must be false"
        );
        hset_free(&se1);
        hset_free(&se2);
    }

    return logret(TEST_PASSED, "done");
}

// ------------------------- TEST 27 ---------------------------------
static TestStatus
tf27(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    /* 1. Перемещение одной строки: оригинал теряет владение */
    test_sub("subtest %d: move single string", ++subnum);
    {
        hset        se = hset_init(10, VALUE64_FS);

        fs          s = fscopy("move_me");          // локальная копия
        //const char *orig_ptr = fsstr(s);       // запомним указатель на строку

        test_validatefree(
            hset_set(&se, value64_movefs(&s)),     // перемещаем!
            hset_free(&se),
            "Failed to insert via FSMOVE"
        );

        // После перемещения оригинал должен быть пуст (не владеть строкой)
        test_validatefree(
            fslen(s) == 0 && fsstr(s) == NULL,
            hset_free(&se),
            "After move, original fs must be empty (len=%d, str=%p)",
            fslen(s), (void*)fsstr(s)
        );

        // Проверяем, что строка во множестве
        fs      search = fsliteral("move_me");
        test_validatefree(
            hset_get(&se, LITERAL64_FS(search)),
            hset_free(&se),
            "Moved string 'move_me' not found in set"
        );

        fsfree(s);      // оригинал пуст, но fsfree безопасен
        hset_free(&se);
    }
    fs_alloc_check(true);
    /* 2. Перемещение нескольких строк */
    test_sub("subtest %d: move multiple strings", ++subnum);
    {
        hset        se = hset_init(10, VALUE64_FS);
        const int   cnt = 20;
        fs          strings[cnt];

        for (int i = 0; i < cnt; i++) {
            strings[i] = FS();
            fs_sprintf(strings + i, "str_%d", i);
            test_validatefree(
                hset_set(&se, value64_movefs(&strings[i])),
                hset_free(&se),
                "Failed to move 'str_%d'", i
            );
            // Оригинал должен быть пуст
            test_validatefree(
                fslen(strings[i]) == 0 && fsstr(strings[i]) == NULL,
                hset_free(&se),
                "After move %d, original must be empty", i
            );
        }

        // Проверяем количество
        test_validatefree(
            hset_cnt(&se) == cnt,
            hset_free(&se),
            "Count after moves: %d, expected %d", hset_cnt(&se), cnt
        );

        // Проверяем наличие всех строк
        for (int i = 0; i < cnt; i++) {
            fs search = FS();
            fs_sprintf(&search, "str_%d", i); 
            test_validatefree(
                hset_get(&se, LITERAL64_FS(search)),
                hset_free(&se),
                "Moved string 'str_%d' not found", i
            );
            fsfree(search);
        }

        // Освобождаем пустые оригиналы
        for (int i = 0; i < cnt; i++)
            fsfree(strings[i]);
        hset_free(&se);
    }
    fs_alloc_check(true);
    /* 3. Попытка переместить уже перемещённый (пустой) fs */
    test_sub("subtest %d: move already moved (empty) fs", ++subnum);
    {
        hset    se = hset_init(10, VALUE64_FS);

        fs      s = fscopy("first_move");
        hset_set(&se, value64_movefs(&s));         // первое перемещение

        // s теперь пуст, попытка ещё раз переместить должна просто не добавить элемент
        // NOT SURE
        bool    added = hset_set(&se, value64_movefs(&s));
        test_validatefree(
            added == false,                     // пустой fs не должен добавиться
            hset_free(&se),
            "Moving empty fs should return false"
        );

        test_validatefree(
            hset_cnt(&se) == 1,
            hset_free(&se),
            "Count after second move attempt: %d, expected 1", hset_cnt(&se)
        );

        fsfree(s);
        hset_free(&se);
    }
    fs_alloc_check(true);
    /* 4. Перемещение с последующим поиском через копию */
    test_sub("subtest %d: move then search by copy", ++subnum);
    {
        hset    se = hset_init(10, VALUE64_FS);

        fs      orig = fscopy("search_me");
        hset_set(&se, value64_movefs(&orig));

        // Поиск через копию (VALUE64_FS делает глубокую копию)
        fs      copy = fscopy("search_me");
        test_validatefree(
            hset_get(&se, LITERAL64_FS(copy)),
            hset_free(&se),
            "Moved string should be found by copy"
        );

        fsfree(copy);
        fsfree(orig);
        hset_free(&se);
    }
    fs_alloc_check(true);
    // ADDITIONAL 
    test_sub("subtest %d: move 5 strings and clone", ++subnum);
    {
        const int cnt = 5;
        // Массив локальных fs – они будут перемещены и опустеют
        fs      strings[cnt];
        const char *expected[] = { "alpha", "beta", "gamma", "delta", "epsilon" };

        // Создаём строки через fscopyf (все в куче, с FS_FLAG_ALLOC)
        for (int i = 0; i < cnt; i++) {
            strings[i] = fscopyf("%s", expected[i]);
        }

        // Множество, в которое будем перемещать
        hset    se = hset_init(cnt, VALUE64_FS);

        // Перемещаем каждую строку (оригиналы опустеют)
        for (int i = 0; i < cnt; i++) {
            test_validatefree(
                hset_set(&se, value64_movefs(&strings[i])) == true,
                hset_free(&se),
                "Failed to move '%s' into set", expected[i]
            );
            // После перемещения strings[i] не должен владеть строкой
            test_validatefree(
                fslen(strings[i]) == 0 && fsstr(strings[i]) == NULL,
                hset_free(&se),
                "After move, original must be empty, but len=%d, str=%p",
                fslen(strings[i]), (void*)fsstr(strings[i])
            );
        }

        // Клонируем множество (внутренние строки должны быть глубоко скопированы)
        hset    clone = hset_clone(&se);

        // Проверяем, что в клоне есть все строки
        for (int i = 0; i < cnt; i++) {
            fs      tmp = fscopyf("%s", expected[i]);   // временный ключ для поиска
            test_validatefree(
                hset_get(&clone, LITERAL64_FS(tmp)),
                (hset_free(&se), hset_free(&clone), fsfree(tmp)),
                "Clone missing '%s'", expected[i]
            );
            fsfree(tmp);
        }

        // Количество элементов должно совпадать
        test_validatefree(
            hset_cnt(&clone) == cnt,
            (hset_free(&se), hset_free(&clone)),
            "Clone count = %d, expected %d", hset_cnt(&clone), cnt
        );

        // Структурная целостность клона
        test_validatefree(
            hset_validate(logfile, &clone),
            (hset_free(&se), hset_free(&clone)),
            "Clone validation failed"
        );

        // Оригинальные strings не нужно освобождать – они пусты после перемещения
        // (fsfree для пустого fs безопасен, но не обязателен)
        hset_free(&se);
        hset_free(&clone);
    }
    fs_alloc_check(true);
    return logret(TEST_PASSED, "done");
}

// ------------------------------------------------------------------------------------------------------------------------------
int
main(int argc, const char *argv[])
{
    logsimpleinit("Start");
    bool    runall = argc == 1;
    printf("%d\n", argc);

    while (runall || *++argv){
        int     num = INT_MAX;    // INT_MAX for all test
        if (!runall){
            num = atoi(*argv);
            if (num < 0){
                fprintf(stderr,"Invalid test num %d\n", num);
                continue;
            }
        }
        printf("Num %d\n", num);
            testenginestd_run(num,
                testnew(.f2 =  tf1,             .num =  1, .name = "Simple init and validate test"              , .desc="", .mandatory=true)
              , testnew(.f2 =  tf2,             .num =  2, .name = "Simple init and add test"                   , .desc="", .mandatory=true)
              , testnew(.f2 =  tf3,             .num =  3, .name = "Simple clone and create from array test"    , .desc="", .mandatory=true)
              , testnew(.f2 =  tf_loadfs_str,   .num =  4, .name = "Simple create fs from c-str array test"     , .desc="", .mandatory=true)
              , testnew(.f2 =  tf4,             .num =  5, .name = "Simple count test"                          , .desc="", .mandatory=true)
              , testnew(.f2 =  tf5,             .num =  6, .name = "Comparation simple test"                    , .desc="", .mandatory=true)
              , testnew(.f2 =  tf6,             .num =  7, .name = "Not equal simple test"                      , .desc="", .mandatory=true)
              , testnew(.f2 =  tf7,             .num =  8, .name = "Cloneas simple test"                        , .desc="", .mandatory=true)
              , testnew(.f2 =  tf8,             .num =  9, .name = "Hset_in simple test"                        , .desc="", .mandatory=true)
              , testnew(.f2 =  tf9,             .num = 10, .name = "Hset_strictin simple test"                  , .desc="", .mandatory=true)
              , testnew(.f2 = tf10,             .num = 11, .name = "Hset_minus simple test"                     , .desc="", .mandatory=true)
              , testnew(.f2 = tf11,             .num = 12, .name = "Hset_init_minus simple test"                , .desc="", .mandatory=true)
              , testnew(.f2 = tf12,             .num = 13, .name = "hset_intersect simple test"                 , .desc="", .mandatory=true)
              , testnew(.f2 = tf13,             .num = 14, .name = "hset_init_intersect simple test"            , .desc="", .mandatory=true)
              , testnew(.f2 = tf14,             .num = 15, .name = "hset_init_symmdiff simple test"             , .desc="", .mandatory=true)
              , testnew(.f2 = tf15,             .num = 16, .name = "hset_symmdiff simple test"                  , .desc="", .mandatory=true)
              , testnew(.f2 = tf16,             .num = 17, .name = "hset_init_resize simple test"               , .desc="", .mandatory=true)
              , testnew(.f2 = tf17,             .num = 18, .name = "hset_union simple test"                     , .desc="", .mandatory=true)
              , testnew(.f2 = tf18,             .num = 19, .name = "hset_init_union simple test"                , .desc="", .mandatory=true)
              , testnew(.f2 = tf19,             .num = 20, .name = "hset_normalize simple test"                 , .desc="", .mandatory=true)
              , testnew(.f2 = tf20,             .num = 21, .name = "hset_save/load simple test"                 , .desc="", .mandatory=true)
              , testnew(.f2 = tf21,             .num = 22, .name = "hset_const_foreach simple test"             , .desc="", .mandatory=true)
              , testnew(.f2 = tf22,             .num = 23, .name = "hset_initreduct int impl  simple test"      , .desc="", .mandatory=true)
              , testnew(.f2 = tf23,             .num = 24, .name = "hset_initreduct double int simple test"     , .desc="", .mandatory=true)
              , testnew(.f2 = tf24,             .num = 25, .name = "inf/nan double int simple test"             , .desc="", .mandatory=true)
              , testnew(.f2 = tf25,             .num = 26, .name = "Macro-base iterator simple test"            , .desc="", .mandatory=true)
              , testnew(.f2 = tf26,             .num = 27, .name = "hset_any(), hset_nonexists() simple test"   , .desc="", .mandatory=true)
              , testnew(.f2 = tf27,             .num = 28, .name = "value64_movefs() simple test"               , .desc="", .mandatory=true)
            );
        if (runall)
            break;
    }

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* HSETTESTING */


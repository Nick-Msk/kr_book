#include <stdarg.h>
#include <stdio.h>
#include <log.h>

static void		va_print(va_list ap, int cnt){
	logenter("cnt = %d", cnt);

	while(cnt--)
		logmsg("... %d", va_arg(ap, int));

	va_end(ap);

	logret(0, "quit");
}

static int		va_check(int cnt, ...){
	logenter("cnt = %d", cnt);
	va_list  	ap, ap_save;
	va_start(ap, cnt);

	int 		n = cnt/2;
	cnt -= n;
	while(n--)
		logmsg("next %d", va_arg(ap, int));
	va_copy(ap_save, ap);

	logmsg("print renains");
	va_print(ap_save, cnt);

	va_end(ap);
	return logret(0, "quit...");	// TODO: logleave this is logret without value
}


int		main(void){
	logenter("...");
	va_check(10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0);
	return logret(0, "");
}

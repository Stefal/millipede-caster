#include <stdio.h>
#include <string.h>

#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include "conf.h"
#include "adm.h"
#include "ntrip_common.h"
#include "ntripsrv.h"

int admsrv(struct ntrip_state *st, const char *root_uri, const char *uri, int *err, struct evkeyvalq *headers) {
	struct evbuffer *output = bufferevent_get_output(st->bev);
#if DEBUG
	if (!st->user || !check_password(st, "admin", st->user, st->password)) {
		int www_auth_value_len = strlen(root_uri) + 15;
		char *www_auth_value = (char *)strmalloc(www_auth_value_len);
		snprintf(www_auth_value, www_auth_value_len, "Basic realm=\"%s\"", root_uri);

		if (!www_auth_value) {
			ntrip_log(st, LOG_CRIT, "ntripsrv: out of memory\n");
			*err = 500;
			evbuffer_add_reference(output, "Out of memory :(\n", 17, NULL, NULL);
			return -1;
		}

		*err = 401;
		evhttp_add_header(headers, "WWW-Authenticate", "Basic realm=\"/adm\"");
		strfree(www_auth_value);
		return 0;
	}
	st->client_version = 2;

	if (!strcmp(uri, "/mem") || !strcmp(uri, "/mem.json")) {
		int len = strlen(uri);
		int json = (len >= 5 && !strcmp(uri+len-5, ".json"))?1:0;

		char *r = malloc_stats_dump(json);
		if (r) {
			ntripsrv_send_result_ok(st, output, "text/plain", NULL);
			if (evbuffer_add_reference(output, r, strlen(r), free_callback, NULL) < 0)
				strfree(r);
		} else {
			ntrip_log(st, LOG_CRIT, "ntripsrv: out of memory\n");
			*err = 500;
			evbuffer_add_reference(output, "Out of memory :(\n", 17, NULL, NULL);
			return -1;
		}
		st->state = NTRIP_WAIT_CLOSE;
		return 0;
	}
#endif
	return 0;
}


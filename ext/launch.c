#include <errno.h>
#include <launch.h>
#include <ruby.h>

VALUE rb_mLaunch;
VALUE rb_mLaunchMessages;
VALUE rb_cLaunchJob;
VALUE rb_cLaunchJobSocket;

static launch_data_t ruby_to_launch_data(VALUE);
static VALUE launch_data_to_ruby(launch_data_t);

static int ruby_to_launch_data_hash_iterator(VALUE key, VALUE value, VALUE dict)
{
  launch_data_dict_insert((launch_data_t)dict, ruby_to_launch_data(value), StringValueCStr(key));
  return ST_CONTINUE;
}

static launch_data_t ruby_to_launch_data(VALUE obj)
{
	launch_data_t result = NULL;
  long i;

	switch(TYPE(obj))
	{
    case T_HASH:
      result = launch_data_alloc(LAUNCH_DATA_DICTIONARY);
      rb_hash_foreach(obj, ruby_to_launch_data_hash_iterator, (VALUE)result);
      break;
    case T_ARRAY:
      result = launch_data_alloc(LAUNCH_DATA_ARRAY);
      for (i = 0; i < RARRAY_LEN(obj); i++)
        launch_data_array_set_index(result, ruby_to_launch_data(RARRAY_PTR(obj)[i]), i);
      break;
    case T_FIXNUM:
    case T_BIGNUM:
      result = launch_data_new_integer(NUM2LONG(obj));
      break;
    case T_FLOAT:
      result = launch_data_new_real(NUM2DBL(obj));
      break;
    case T_TRUE:
      result = launch_data_new_bool(TRUE);
      break;
    case T_FALSE:
      result = launch_data_new_bool(FALSE);
      break;
		case T_STRING:
			result = launch_data_new_string(StringValueCStr(obj));
      break;
		default:
			rb_raise(rb_eTypeError, "can't convert %s to launch data", rb_obj_classname(obj));
	}

	return result;
}

static void launch_data_to_ruby_dict_iterator(launch_data_t item, const char * key, void * hash)
{
	rb_hash_aset((VALUE)hash, rb_str_new2(key), launch_data_to_ruby(item));
}

static VALUE launch_data_to_ruby(launch_data_t item)
{
	VALUE result = Qnil;
	size_t i, count;

	switch(launch_data_get_type(item))
	{
		case LAUNCH_DATA_DICTIONARY:
			result = rb_hash_new();
			launch_data_dict_iterate(item, launch_data_to_ruby_dict_iterator, (void *)result);
			break;
		case LAUNCH_DATA_ARRAY:
			count = launch_data_array_get_count(item);
			result = rb_ary_new2(count);
			for (i = 0; i < count; i++)
				rb_ary_store(result, i, launch_data_to_ruby(launch_data_array_get_index(item, i)));
			break;
		case LAUNCH_DATA_FD:
      // TODO: use rb_io_s_for_fd
			result = INT2NUM(launch_data_get_fd(item));
			break;
		case LAUNCH_DATA_INTEGER:
			result = LONG2NUM(launch_data_get_integer(item));
			break;
		case LAUNCH_DATA_REAL:
			result = DBL2NUM(launch_data_get_real(item));
			break;
		case LAUNCH_DATA_BOOL:
			result = launch_data_get_bool(item) == TRUE ? Qtrue : Qfalse;
			break;
		case LAUNCH_DATA_STRING:
			result = rb_str_new2(launch_data_get_string(item));
			break;
		case LAUNCH_DATA_OPAQUE:
			result = rb_str_new(launch_data_get_opaque(item), launch_data_get_opaque_size(item));
			break;
		case LAUNCH_DATA_ERRNO:
      errno = launch_data_get_errno(item);

      if (errno)
        rb_sys_fail("launch");
      else
        result = Qtrue;
			break;
		case LAUNCH_DATA_MACHPORT:
			break;
    default:
      rb_warn("unknown launch_data_type_t %d", launch_data_get_type(item));
      break;
	}

	return result;
}

static VALUE launch_message(VALUE self, VALUE obj)
{
	VALUE result = Qnil;
	launch_data_t item, response;

	item = ruby_to_launch_data(obj);

  response = launch_msg(item);
  launch_data_free(item);

	if (response == NULL)
		rb_sys_fail("launch");

	result = launch_data_to_ruby(response);
	launch_data_free(response);

	return result;
}


void Init_launch()
{
  rb_mLaunch = rb_define_module("Launch");
	rb_define_singleton_method(rb_mLaunch, "message", launch_message, 1);

  rb_mLaunchMessages = rb_define_module_under(rb_mLaunch, "Messages");
  rb_const_set(rb_mLaunchMessages, rb_intern("SUBMITJOB"), rb_str_new2(LAUNCH_KEY_SUBMITJOB));
  rb_const_set(rb_mLaunchMessages, rb_intern("REMOVEJOB"), rb_str_new2(LAUNCH_KEY_REMOVEJOB));
  rb_const_set(rb_mLaunchMessages, rb_intern("STARTJOB"),  rb_str_new2(LAUNCH_KEY_STARTJOB));
  rb_const_set(rb_mLaunchMessages, rb_intern("STOPJOB"),   rb_str_new2(LAUNCH_KEY_STOPJOB));
  rb_const_set(rb_mLaunchMessages, rb_intern("GETJOB"),    rb_str_new2(LAUNCH_KEY_GETJOB));
  rb_const_set(rb_mLaunchMessages, rb_intern("GETJOBS"),   rb_str_new2(LAUNCH_KEY_GETJOBS));
  rb_const_set(rb_mLaunchMessages, rb_intern("CHECKIN"),   rb_str_new2(LAUNCH_KEY_CHECKIN));

  rb_cLaunchJob = rb_define_class_under(rb_mLaunch, "Job", rb_cObject);
  rb_const_set(rb_cLaunchJob, rb_intern("LABEL"),                        rb_str_new2(LAUNCH_JOBKEY_LABEL));
  rb_const_set(rb_cLaunchJob, rb_intern("DISABLED"),                     rb_str_new2(LAUNCH_JOBKEY_DISABLED));
  rb_const_set(rb_cLaunchJob, rb_intern("USERNAME"),                     rb_str_new2(LAUNCH_JOBKEY_USERNAME));
  rb_const_set(rb_cLaunchJob, rb_intern("GROUPNAME"),                    rb_str_new2(LAUNCH_JOBKEY_GROUPNAME));
  rb_const_set(rb_cLaunchJob, rb_intern("TIMEOUT"),                      rb_str_new2(LAUNCH_JOBKEY_TIMEOUT));
  rb_const_set(rb_cLaunchJob, rb_intern("EXITTIMEOUT"),                  rb_str_new2(LAUNCH_JOBKEY_EXITTIMEOUT));
  rb_const_set(rb_cLaunchJob, rb_intern("INITGROUPS"),                   rb_str_new2(LAUNCH_JOBKEY_INITGROUPS));
  rb_const_set(rb_cLaunchJob, rb_intern("SOCKETS"),                      rb_str_new2(LAUNCH_JOBKEY_SOCKETS));
  rb_const_set(rb_cLaunchJob, rb_intern("MACHSERVICES"),                 rb_str_new2(LAUNCH_JOBKEY_MACHSERVICES));
  rb_const_set(rb_cLaunchJob, rb_intern("MACHSERVICELOOKUPPOLICIES"),    rb_str_new2(LAUNCH_JOBKEY_MACHSERVICELOOKUPPOLICIES));
  rb_const_set(rb_cLaunchJob, rb_intern("INETDCOMPATIBILITY"),           rb_str_new2(LAUNCH_JOBKEY_INETDCOMPATIBILITY));
  rb_const_set(rb_cLaunchJob, rb_intern("ENABLEGLOBBING"),               rb_str_new2(LAUNCH_JOBKEY_ENABLEGLOBBING));
  rb_const_set(rb_cLaunchJob, rb_intern("PROGRAMARGUMENTS"),             rb_str_new2(LAUNCH_JOBKEY_PROGRAMARGUMENTS));
  rb_const_set(rb_cLaunchJob, rb_intern("PROGRAM"),                      rb_str_new2(LAUNCH_JOBKEY_PROGRAM));
  rb_const_set(rb_cLaunchJob, rb_intern("ONDEMAND"),                     rb_str_new2(LAUNCH_JOBKEY_ONDEMAND));
  rb_const_set(rb_cLaunchJob, rb_intern("KEEPALIVE"),                    rb_str_new2(LAUNCH_JOBKEY_KEEPALIVE));
  rb_const_set(rb_cLaunchJob, rb_intern("LIMITLOADTOHOSTS"),             rb_str_new2(LAUNCH_JOBKEY_LIMITLOADTOHOSTS));
  rb_const_set(rb_cLaunchJob, rb_intern("LIMITLOADFROMHOSTS"),           rb_str_new2(LAUNCH_JOBKEY_LIMITLOADFROMHOSTS));
  rb_const_set(rb_cLaunchJob, rb_intern("LIMITLOADTOSESSIONTYPE"),       rb_str_new2(LAUNCH_JOBKEY_LIMITLOADTOSESSIONTYPE));
  rb_const_set(rb_cLaunchJob, rb_intern("RUNATLOAD"),                    rb_str_new2(LAUNCH_JOBKEY_RUNATLOAD));
  rb_const_set(rb_cLaunchJob, rb_intern("ROOTDIRECTORY"),                rb_str_new2(LAUNCH_JOBKEY_ROOTDIRECTORY));
  rb_const_set(rb_cLaunchJob, rb_intern("WORKINGDIRECTORY"),             rb_str_new2(LAUNCH_JOBKEY_WORKINGDIRECTORY));
  rb_const_set(rb_cLaunchJob, rb_intern("ENVIRONMENTVARIABLES"),         rb_str_new2(LAUNCH_JOBKEY_ENVIRONMENTVARIABLES));
  rb_const_set(rb_cLaunchJob, rb_intern("USERENVIRONMENTVARIABLES"),     rb_str_new2(LAUNCH_JOBKEY_USERENVIRONMENTVARIABLES));
  rb_const_set(rb_cLaunchJob, rb_intern("UMASK"),                        rb_str_new2(LAUNCH_JOBKEY_UMASK));
  rb_const_set(rb_cLaunchJob, rb_intern("NICE"),                         rb_str_new2(LAUNCH_JOBKEY_NICE));
  rb_const_set(rb_cLaunchJob, rb_intern("HOPEFULLYEXITSFIRST"),          rb_str_new2(LAUNCH_JOBKEY_HOPEFULLYEXITSFIRST));
  rb_const_set(rb_cLaunchJob, rb_intern("HOPEFULLYEXITSLAST"),           rb_str_new2(LAUNCH_JOBKEY_HOPEFULLYEXITSLAST));
  rb_const_set(rb_cLaunchJob, rb_intern("LOWPRIORITYIO"),                rb_str_new2(LAUNCH_JOBKEY_LOWPRIORITYIO));
  rb_const_set(rb_cLaunchJob, rb_intern("SESSIONCREATE"),                rb_str_new2(LAUNCH_JOBKEY_SESSIONCREATE));
  rb_const_set(rb_cLaunchJob, rb_intern("STARTONMOUNT"),                 rb_str_new2(LAUNCH_JOBKEY_STARTONMOUNT));
  rb_const_set(rb_cLaunchJob, rb_intern("SOFTRESOURCELIMITS"),           rb_str_new2(LAUNCH_JOBKEY_SOFTRESOURCELIMITS));
  rb_const_set(rb_cLaunchJob, rb_intern("HARDRESOURCELIMITS"),           rb_str_new2(LAUNCH_JOBKEY_HARDRESOURCELIMITS));
  rb_const_set(rb_cLaunchJob, rb_intern("STANDARDINPATH"),               rb_str_new2(LAUNCH_JOBKEY_STANDARDINPATH));
  rb_const_set(rb_cLaunchJob, rb_intern("STANDARDOUTPATH"),              rb_str_new2(LAUNCH_JOBKEY_STANDARDOUTPATH));
  rb_const_set(rb_cLaunchJob, rb_intern("STANDARDERRORPATH"),            rb_str_new2(LAUNCH_JOBKEY_STANDARDERRORPATH));
  rb_const_set(rb_cLaunchJob, rb_intern("DEBUG"),                        rb_str_new2(LAUNCH_JOBKEY_DEBUG));
  rb_const_set(rb_cLaunchJob, rb_intern("WAITFORDEBUGGER"),              rb_str_new2(LAUNCH_JOBKEY_WAITFORDEBUGGER));
  rb_const_set(rb_cLaunchJob, rb_intern("QUEUEDIRECTORIES"),             rb_str_new2(LAUNCH_JOBKEY_QUEUEDIRECTORIES));
  rb_const_set(rb_cLaunchJob, rb_intern("WATCHPATHS"),                   rb_str_new2(LAUNCH_JOBKEY_WATCHPATHS));
  rb_const_set(rb_cLaunchJob, rb_intern("STARTINTERVAL"),                rb_str_new2(LAUNCH_JOBKEY_STARTINTERVAL));
  rb_const_set(rb_cLaunchJob, rb_intern("STARTCALENDARINTERVAL"),        rb_str_new2(LAUNCH_JOBKEY_STARTCALENDARINTERVAL));
  rb_const_set(rb_cLaunchJob, rb_intern("BONJOURFDS"),                   rb_str_new2(LAUNCH_JOBKEY_BONJOURFDS));
  rb_const_set(rb_cLaunchJob, rb_intern("LASTEXITSTATUS"),               rb_str_new2(LAUNCH_JOBKEY_LASTEXITSTATUS));
  rb_const_set(rb_cLaunchJob, rb_intern("PID"),                          rb_str_new2(LAUNCH_JOBKEY_PID));
  rb_const_set(rb_cLaunchJob, rb_intern("THROTTLEINTERVAL"),             rb_str_new2(LAUNCH_JOBKEY_THROTTLEINTERVAL));
  rb_const_set(rb_cLaunchJob, rb_intern("LAUNCHONLYONCE"),               rb_str_new2(LAUNCH_JOBKEY_LAUNCHONLYONCE));
  rb_const_set(rb_cLaunchJob, rb_intern("ABANDONPROCESSGROUP"),          rb_str_new2(LAUNCH_JOBKEY_ABANDONPROCESSGROUP));
  rb_const_set(rb_cLaunchJob, rb_intern("IGNOREPROCESSGROUPATSHUTDOWN"), rb_str_new2(LAUNCH_JOBKEY_IGNOREPROCESSGROUPATSHUTDOWN));
  rb_const_set(rb_cLaunchJob, rb_intern("POLICIES"),                     rb_str_new2(LAUNCH_JOBKEY_POLICIES));
  rb_const_set(rb_cLaunchJob, rb_intern("ENABLETRANSACTIONS"),           rb_str_new2(LAUNCH_JOBKEY_ENABLETRANSACTIONS));
  rb_const_set(rb_cLaunchJob, rb_intern("POLICY_DENYCREATINGOTHERJOBS"), rb_str_new2(LAUNCH_JOBPOLICY_DENYCREATINGOTHERJOBS));
  rb_const_set(rb_cLaunchJob, rb_intern("INETDCOMPATIBILITY_WAIT"),      rb_str_new2(LAUNCH_JOBINETDCOMPATIBILITY_WAIT));
  rb_const_set(rb_cLaunchJob, rb_intern("MACH_RESETATCLOSE"),            rb_str_new2(LAUNCH_JOBKEY_MACH_RESETATCLOSE));
  rb_const_set(rb_cLaunchJob, rb_intern("MACH_HIDEUNTILCHECKIN"),        rb_str_new2(LAUNCH_JOBKEY_MACH_HIDEUNTILCHECKIN));
  rb_const_set(rb_cLaunchJob, rb_intern("MACH_DRAINMESSAGESONCRASH"),    rb_str_new2(LAUNCH_JOBKEY_MACH_DRAINMESSAGESONCRASH));
  rb_const_set(rb_cLaunchJob, rb_intern("KEEPALIVE_SUCCESSFULEXIT"),     rb_str_new2(LAUNCH_JOBKEY_KEEPALIVE_SUCCESSFULEXIT));
  rb_const_set(rb_cLaunchJob, rb_intern("KEEPALIVE_NETWORKSTATE"),       rb_str_new2(LAUNCH_JOBKEY_KEEPALIVE_NETWORKSTATE));
  rb_const_set(rb_cLaunchJob, rb_intern("KEEPALIVE_PATHSTATE"),          rb_str_new2(LAUNCH_JOBKEY_KEEPALIVE_PATHSTATE));
  rb_const_set(rb_cLaunchJob, rb_intern("KEEPALIVE_OTHERJOBACTIVE"),     rb_str_new2(LAUNCH_JOBKEY_KEEPALIVE_OTHERJOBACTIVE));
  rb_const_set(rb_cLaunchJob, rb_intern("KEEPALIVE_OTHERJOBENABLED"),    rb_str_new2(LAUNCH_JOBKEY_KEEPALIVE_OTHERJOBENABLED));
  rb_const_set(rb_cLaunchJob, rb_intern("KEEPALIVE_AFTERINITIALDEMAND"), rb_str_new2(LAUNCH_JOBKEY_KEEPALIVE_AFTERINITIALDEMAND));
  rb_const_set(rb_cLaunchJob, rb_intern("CAL_MINUTE"),                   rb_str_new2(LAUNCH_JOBKEY_CAL_MINUTE));
  rb_const_set(rb_cLaunchJob, rb_intern("CAL_HOUR"),                     rb_str_new2(LAUNCH_JOBKEY_CAL_HOUR));
  rb_const_set(rb_cLaunchJob, rb_intern("CAL_DAY"),                      rb_str_new2(LAUNCH_JOBKEY_CAL_DAY));
  rb_const_set(rb_cLaunchJob, rb_intern("CAL_WEEKDAY"),                  rb_str_new2(LAUNCH_JOBKEY_CAL_WEEKDAY));
  rb_const_set(rb_cLaunchJob, rb_intern("CAL_MONTH"),                    rb_str_new2(LAUNCH_JOBKEY_CAL_MONTH));
  rb_const_set(rb_cLaunchJob, rb_intern("RESOURCELIMIT_CORE"),           rb_str_new2(LAUNCH_JOBKEY_RESOURCELIMIT_CORE));
  rb_const_set(rb_cLaunchJob, rb_intern("RESOURCELIMIT_CPU"),            rb_str_new2(LAUNCH_JOBKEY_RESOURCELIMIT_CPU));
  rb_const_set(rb_cLaunchJob, rb_intern("RESOURCELIMIT_DATA"),           rb_str_new2(LAUNCH_JOBKEY_RESOURCELIMIT_DATA));
  rb_const_set(rb_cLaunchJob, rb_intern("RESOURCELIMIT_FSIZE"),          rb_str_new2(LAUNCH_JOBKEY_RESOURCELIMIT_FSIZE));
  rb_const_set(rb_cLaunchJob, rb_intern("RESOURCELIMIT_MEMLOCK"),        rb_str_new2(LAUNCH_JOBKEY_RESOURCELIMIT_MEMLOCK));
  rb_const_set(rb_cLaunchJob, rb_intern("RESOURCELIMIT_NOFILE"),         rb_str_new2(LAUNCH_JOBKEY_RESOURCELIMIT_NOFILE));
  rb_const_set(rb_cLaunchJob, rb_intern("RESOURCELIMIT_NPROC"),          rb_str_new2(LAUNCH_JOBKEY_RESOURCELIMIT_NPROC));
  rb_const_set(rb_cLaunchJob, rb_intern("RESOURCELIMIT_RSS"),            rb_str_new2(LAUNCH_JOBKEY_RESOURCELIMIT_RSS));
  rb_const_set(rb_cLaunchJob, rb_intern("RESOURCELIMIT_STACK"),          rb_str_new2(LAUNCH_JOBKEY_RESOURCELIMIT_STACK));
  rb_const_set(rb_cLaunchJob, rb_intern("DISABLED_MACHINETYPE"),         rb_str_new2(LAUNCH_JOBKEY_DISABLED_MACHINETYPE));
  rb_const_set(rb_cLaunchJob, rb_intern("DISABLED_MODELNAME"),           rb_str_new2(LAUNCH_JOBKEY_DISABLED_MODELNAME));

  rb_require("socket");
  rb_cLaunchJobSocket = rb_define_class_under(rb_cLaunchJob, "Socket", rb_path2class("TCPSocket"));
  rb_const_set(rb_cLaunchJobSocket, rb_intern("TYPE"),           rb_str_new2(LAUNCH_JOBSOCKETKEY_TYPE));
  rb_const_set(rb_cLaunchJobSocket, rb_intern("PASSIVE"),        rb_str_new2(LAUNCH_JOBSOCKETKEY_PASSIVE));
  rb_const_set(rb_cLaunchJobSocket, rb_intern("BONJOUR"),        rb_str_new2(LAUNCH_JOBSOCKETKEY_BONJOUR));
  rb_const_set(rb_cLaunchJobSocket, rb_intern("SECUREWITHKEY"),  rb_str_new2(LAUNCH_JOBSOCKETKEY_SECUREWITHKEY));
  rb_const_set(rb_cLaunchJobSocket, rb_intern("PATHNAME"),       rb_str_new2(LAUNCH_JOBSOCKETKEY_PATHNAME));
  rb_const_set(rb_cLaunchJobSocket, rb_intern("PATHMODE"),       rb_str_new2(LAUNCH_JOBSOCKETKEY_PATHMODE));
  rb_const_set(rb_cLaunchJobSocket, rb_intern("NODENAME"),       rb_str_new2(LAUNCH_JOBSOCKETKEY_NODENAME));
  rb_const_set(rb_cLaunchJobSocket, rb_intern("SERVICENAME"),    rb_str_new2(LAUNCH_JOBSOCKETKEY_SERVICENAME));
  rb_const_set(rb_cLaunchJobSocket, rb_intern("FAMILY"),         rb_str_new2(LAUNCH_JOBSOCKETKEY_FAMILY));
  rb_const_set(rb_cLaunchJobSocket, rb_intern("PROTOCOL"),       rb_str_new2(LAUNCH_JOBSOCKETKEY_PROTOCOL));
  rb_const_set(rb_cLaunchJobSocket, rb_intern("MULTICASTGROUP"), rb_str_new2(LAUNCH_JOBSOCKETKEY_MULTICASTGROUP));
}

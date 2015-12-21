#include "DECAF_types.h"
#include "DECAF_main.h"
#include "DECAF_callback.h"
#include "DECAF_callback_common.h"
#include "vmi_callback.h"
#include "utils/Output.h"
#include "DECAF_target.h"
  
//basic stub for plugins
static plugin_interface_t my_interface;
static DECAF_Handle loadmainmodule_handle = DECAF_NULL_HANDLE;
static DECAF_Handle blockbegin_handle = DECAF_NULL_HANDLE;
static DECAF_Handle blockend_handle = DECAF_NULL_HANDLE;

char targetname[512];
char mesg[512];
uint32_t target_cr3;
FILE *fp;
uint32_t limit;
uint32_t base;

static uint32_t start_addr = 0; 
#define BUFMAX 1024

static void trace_block_end_callback(DECAF_Callback_Params* params)
{
	uint32_t pre = params->be.cur_pc;
	uint32_t next = params->be.next_pc;
	unsigned char buf[BUFMAX];
	int size;
	int i;
	if(params->be.env->cr[3]==target_cr3 &&  pre>=base && pre <=limit  )
        {
		//fprintf(fp, "Env->eip %08x\n",params->be.env->eip);
		
		//fprintf(fp, "Cur_pc %08x next_pc %08x\n",params->be.cur_pc-base, params->be.next_pc-base);
		fprintf(fp, "End Block: 0x%08x 0x%0x08x\n", pre, next);
		if(start_addr!=0 && pre >= start_addr)
		{
			size = (pre-start_addr+10 > BUFMAX)? BUFMAX : pre-start_addr+10;
			DECAF_read_mem(params->be.env , start_addr, size , buf);
			for(i = 0; i< size && i< BUFMAX; i++)
			{
				fprintf(fp, "%02x", buf[i]);
			}
			fprintf(fp, "\n");
		}
	     //DECAF_printf("Env->eip %08x\n",params->be.env->eip);
     	     //DECAF_printf("Cur_pc %08x next_pc %08x\n",params->be.cur_pc, params->be.next_pc);
	}
}  

static void trace_block_start_callback(DECAF_Callback_Params* params)
{
        uint32_t pc = params->bb.env->eip;
        if(params->bb.env->cr[3]==target_cr3 && pc>=base && pc <= limit )
        {
		start_addr = pc;
		fprintf(fp, "Begin Block: 0x%08x\n", pc); 
        }
}

/*
* This callback is invoked when a new process starts in the guest OS.
*/
static void my_loadmainmodule_callback(VMI_Callback_Params* params)
{
    if(strcmp(params->lm.name,targetname)==0){
		fprintf(fp, "Process %s you spcecified starts \n",params->lm.name);
		fprintf(fp, "Base %08x Size %x \n",params->lm.base, params->lm.size);
		base = params->lm.base;
		limit = params->lm.base + params->lm.size; 
          DECAF_printf("Process %s you spcecified starts \n",params->lm.name);
	  DECAF_printf("Base %08x Size %x \n",params->lm.base, params->lm.size);
    	  target_cr3=params->cp.cr3;  
          DECAF_register_callback(DECAF_BLOCK_BEGIN_CB ,&trace_block_start_callback,NULL);
     	  DECAF_register_callback(DECAF_BLOCK_END_CB ,&trace_block_end_callback,NULL);
    }
}
/*
 * Handler to implement the command monitor_proc.
*/
  
void do_monitor_proc(Monitor* mon, const QDict* qdict)
{
        /*
         * Copy the name of the process to be monitored to targetname.
         */
    if ((qdict != NULL) && (qdict_haskey(qdict, "procname"))) {
      strncpy(targetname, qdict_get_str(qdict, "procname"), 512);
    }
  targetname[511] = '\0';
}
  
static int my_init(void)
{
    fp = fopen("/home/dsns/vmlog/btrace", "w");
    if(fp==NULL)
	return (0);
    //register for process create and process remove events  
    //blockbegin_handle = DECAF_register_callback( DECAF_BLOCK_BEGIN_CB , &my_loadmainmodule_callback, NULL);
    //if (blockbegin_handle == DECAF_NULL_HANDLE)
    //{
    //  DECAF_printf("Could not register for the create or remove proc events\n");  
    //}
    loadmainmodule_handle = VMI_register_callback( VMI_LOADMODULE_CB , &my_loadmainmodule_callback, NULL);
    if (loadmainmodule_handle == DECAF_NULL_HANDLE)
    {
      DECAF_printf("Could not register for the create or remove proc events\n");
    }
    return (0);
}
  
/*
 * This function is invoked when the plugin is unloaded.
*/
static void my_cleanup(void)
{
    DECAF_printf("Bye world\n");
    /*
     * Unregister for the process start and exit callbacks.
     */
    if (blockbegin_handle != DECAF_NULL_HANDLE) {
      VMI_unregister_callback(DECAF_BLOCK_BEGIN_CB, blockbegin_handle);  
      blockbegin_handle = DECAF_NULL_HANDLE;
    }
    if (blockend_handle != DECAF_NULL_HANDLE) {
      VMI_unregister_callback(DECAF_BLOCK_END_CB, blockend_handle);
      blockend_handle = DECAF_NULL_HANDLE;
    }
    if (loadmainmodule_handle!= DECAF_NULL_HANDLE){
	VMI_unregister_callback(DECAF_BLOCK_END_CB, loadmainmodule_handle);
	loadmainmodule_handle = DECAF_NULL_HANDLE;
    }
	fclose(fp);
} 
/*
 * Commands supported by the plugin. Included in plugin_cmds.h
*/
static mon_cmd_t my_term_cmds[] = {
      {
                        .name           = "monitor_proc",
                        .args_type      = "procname:s?",
                        .mhandler.cmd   = do_monitor_proc,
                        .params         = "[procname]",
                        .help           = "Run the tests with program [procname]"
                },
    {NULL, NULL, },
};
  
  /*
   * This function registers the plugin_interface with DECAF.
   * The interface is used to register custom commands, let DECAF know which cleanup function to call upon plugin unload, etc,.
   */
plugin_interface_t* init_plugin(void)
{
    my_interface.mon_cmds = my_term_cmds;
    my_interface.plugin_cleanup = &my_cleanup;
    my_init();
    return (&my_interface);
}

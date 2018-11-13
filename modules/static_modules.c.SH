#!/bin/sh
SYMS=`for x in $*; do basename $x .o|sed -es/^m_//; done`
cat > static_modules.c <<EOF
#include "stdinc.h"
#include "modules.h"
#include "hash.h"
#include "s_log.h"

EOF

for x in $SYMS; do 
	echo extern struct mapi_header_av1 "$x"_mheader\;
done >> static_modules.c


echo static const struct mapi_header_av1 *mapi_headers[] = { >> static_modules.c 
for x in $SYMS; do 
	echo \&"$x"_mheader,
done >> static_modules.c

echo NULL }\; >> static_modules.c

cat >> static_modules.c <<EOF
void load_static_modules(void)
{
	int x;
	int *mapi_version;
	for(x = 0; mapi_headers[x] != NULL; x++)
	{
		mapi_version = (int *)mapi_headers[x];
		if(MAPI_MAGIC(*mapi_version) != MAPI_MAGIC_HDR)
		{
			ilog(L_MAIN, "Error: linked in module without a MAPI header..giving up");
			exit(70);
		} 	
		switch(MAPI_VERSION(*mapi_version))
		{
			case 1:
			{
				struct mapi_mheader_av1 *mheader = (struct mapi_mheader_av1*)mapi_version;
				if (mheader->mapi_register && (mheader->mapi_register() == -1))
				{
					ilog(L_MAIN, "Error: linked in module failed loading..giving up");
					exit(70);
				}
				
				if(mheader->mapi_command_list)
				{
					struct Message **m;
					for(m = mheader->mapi_command_list; *m; ++m)
						mod_add_cmd(*m);
				}
				
				if(mheader->mapi_hook_list)
				{
					mapi_hlist_av1 *m;
					for(m = mheader->mapi_hook_list; m->hapi_name; ++m)
						*m->hapi_id = register_hook(m->hapi_name);
				}	
				
				if(mheader->mapi_hfn_list)
				{
					mapi_hfn_list_av1 *m;
					for(m = mheader->mapi_hfn_list; m->hapi_name; ++m)
						add_hook(m->hapi_name, m->fn);
						
				}
				
				break;
			
			}	
			default:
			{
				ilog(L_MAIN, "Error: Unknown MAPI version in linked in module..giving up");
				exit(70);
			}				
		}
	}
}
EOF

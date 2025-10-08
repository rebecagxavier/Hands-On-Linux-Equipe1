#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xcb8b6ec6, "kfree" },
	{ 0xdd6830c7, "sprintf" },
	{ 0x43a349ca, "strlen" },
	{ 0x6a939375, "usb_bulk_msg" },
	{ 0x17545440, "strstr" },
	{ 0x67628f51, "msleep" },
	{ 0x173ec8da, "sscanf" },
	{ 0xd272d446, "__stack_chk_fail" },
	{ 0x888b8f57, "strcmp" },
	{ 0x1f55c5b2, "kstrtoll" },
	{ 0x0e1a875a, "kernel_kobj" },
	{ 0xe90293c6, "kobject_create_and_add" },
	{ 0xed75f9ac, "sysfs_create_group" },
	{ 0x2bd64ad0, "usb_find_common_endpoints" },
	{ 0xd710adbf, "__kmalloc_noprof" },
	{ 0x056c43c7, "usb_deregister" },
	{ 0xd272d446, "__fentry__" },
	{ 0x8134d220, "usb_register_driver" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0xe8213e80, "_printk" },
	{ 0x91421947, "kobject_put" },
	{ 0x70eca2ca, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0xcb8b6ec6,
	0xdd6830c7,
	0x43a349ca,
	0x6a939375,
	0x17545440,
	0x67628f51,
	0x173ec8da,
	0xd272d446,
	0x888b8f57,
	0x1f55c5b2,
	0x0e1a875a,
	0xe90293c6,
	0xed75f9ac,
	0x2bd64ad0,
	0xd710adbf,
	0x056c43c7,
	0xd272d446,
	0x8134d220,
	0xd272d446,
	0xe8213e80,
	0x91421947,
	0x70eca2ca,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"kfree\0"
	"sprintf\0"
	"strlen\0"
	"usb_bulk_msg\0"
	"strstr\0"
	"msleep\0"
	"sscanf\0"
	"__stack_chk_fail\0"
	"strcmp\0"
	"kstrtoll\0"
	"kernel_kobj\0"
	"kobject_create_and_add\0"
	"sysfs_create_group\0"
	"usb_find_common_endpoints\0"
	"__kmalloc_noprof\0"
	"usb_deregister\0"
	"__fentry__\0"
	"usb_register_driver\0"
	"__x86_return_thunk\0"
	"_printk\0"
	"kobject_put\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");

MODULE_ALIAS("usb:v1A86p7523d*dc*dsc*dp*ic*isc*ip*in*");

MODULE_INFO(srcversion, "DD41BB227A00AB1412F01FC");

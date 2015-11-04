#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x6d289d41, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x9ecdae3, __VMLINUX_SYMBOL_STR(STUB_start_elevator) },
	{ 0x983489cd, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0xf9a482f9, __VMLINUX_SYMBOL_STR(msleep) },
	{ 0x754d539c, __VMLINUX_SYMBOL_STR(strlen) },
	{ 0x1f3447a5, __VMLINUX_SYMBOL_STR(remove_proc_entry) },
	{ 0x771c958b, __VMLINUX_SYMBOL_STR(mutex_unlock) },
	{ 0xbb04eae1, __VMLINUX_SYMBOL_STR(kthread_create_on_node) },
	{ 0x811606d6, __VMLINUX_SYMBOL_STR(STUB_issue_request) },
	{ 0xfdaaf2c5, __VMLINUX_SYMBOL_STR(mutex_lock_interruptible) },
	{ 0x3f476222, __VMLINUX_SYMBOL_STR(__mutex_init) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x507e09c4, __VMLINUX_SYMBOL_STR(kthread_stop) },
	{ 0xdf9b386b, __VMLINUX_SYMBOL_STR(STUB_stop_elevator) },
	{ 0xdbe7d4fd, __VMLINUX_SYMBOL_STR(wake_up_process) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
	{ 0xc170e366, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0xb3f7646e, __VMLINUX_SYMBOL_STR(kthread_should_stop) },
	{ 0xbc8af77f, __VMLINUX_SYMBOL_STR(proc_create_data) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x69acdf38, __VMLINUX_SYMBOL_STR(memcpy) },
	{ 0x28318305, __VMLINUX_SYMBOL_STR(snprintf) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "B893DE93E9DCD3F23FDA905");

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/cpu.h>
#include <linux/slab.h>
#include <asm/msr.h>
#include <asm/processor.h>

#define CPUID_EDX_MSR_SUPPORT (1 << 5)

struct msr_def {
    u32 addr;
    char *name;
    struct kobj_attribute attr;
};

struct msr_kobj_container {
	u8 valid;
	struct kobject kobj;
	struct device *cpu;
};

static struct kobj_type sysfs_msrs_ktype = {
	.sysfs_ops = &kobj_sysfs_ops
};

static DEFINE_PER_CPU(struct msr_kobj_container*, msr_kobjs);

#define MSR_FUNCS(_addr, _name) \
	static ssize_t show_msr_##_name(struct kobject *kobj, struct kobj_attribute *attr, char *buf) { \
		struct msr_kobj_container *c = container_of(kobj, struct msr_kobj_container, kobj); \
		u32 l, h; \
		\
		if(rdmsr_safe_on_cpu(c->cpu->id, _addr, &l, &h)) { \
			return -EIO; \
		} \
		\
		if(h != 0) \
			return sprintf(buf, "%x%x\n", h, l); \
		else \
			return sprintf(buf, "%x\n", l); \
	} \
	\
	static ssize_t store_msr_##_name(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) { \
		struct msr_kobj_container *c = container_of(kobj, struct msr_kobj_container, kobj); \
		u64 v; \
		int ret; \
		\
		ret = kstrtou64(buf, 0, &v); \
		if(ret) \
			return ret; \
		\
		if(wrmsr_safe_on_cpu(c->cpu->id, _addr, (u32) (v & 0xFFFFFFFFll), (u32) ((v & 0xFFFFFFFF00000000ll) >> 4))) { \
			return -EIO; \
		} \
		\
		return count; \
	}

#include "msrs.h"

static void _deref_kobjs(void) {
	int i;
	for(i = 0; i < NR_CPUS; i++) {
		struct msr_kobj_container *c = per_cpu(msr_kobjs, i);
		if(c->valid)
			kobject_put(&c->kobj);
	}
}

static int __init sysfs_msrs_init(void) {
	unsigned int eax, ebx, ecx, edx;
	int i;

	cpuid(0x01, &eax, &ebx, &ecx, &edx);
	if((edx & CPUID_EDX_MSR_SUPPORT) == 0) {
		pr_warn("RDMSR not supported\n");
		return -ENODEV;
	}

	for(i = 0; i < NR_CPUS; i++) {
		per_cpu(msr_kobjs, i) = kzalloc(sizeof(struct msr_kobj_container), GFP_KERNEL);
	}

	for(i = 0; i < NR_CPUS; i++) {
		int j;
		struct msr_kobj_container *c = per_cpu(msr_kobjs, i);
		struct device *cpu = get_cpu_device(i);

		if(cpu == NULL) {
			continue;
		}

		c->cpu = cpu;

		if(kobject_init_and_add(&c->kobj, &sysfs_msrs_ktype, &cpu->kobj, "msrs")) {
			_deref_kobjs();
			return -ENOMEM;
		}

		c->valid = 1;

		for(j = 0;; j++) {
			u32 l, h;
			struct msr_def *msr = &msrs[j];
			if(msr->addr == 0x0)
				break;

			if(rdmsr_safe_on_cpu(i, msr->addr, &l, &h)) {
				continue;
			}

			if(sysfs_create_file(&c->kobj, &msr->attr.attr)) {
				_deref_kobjs();
				return -ENOMEM;
			}
		}
	}

	return 0;
}
module_init(sysfs_msrs_init);

static void __exit sysfs_msrs_exit(void) {
	_deref_kobjs();
}
module_exit(sysfs_msrs_exit);

MODULE_AUTHOR("Yussuf Khalil <dev@pp3345.net>");
MODULE_DESCRIPTION("Expose x86 CPU MSRs to sysfs");
MODULE_LICENSE("GPL");

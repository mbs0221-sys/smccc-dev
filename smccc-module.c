#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/arm-smccc.h>
#include <linux/time.h>
#include <linux/ktime.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include <asm/archrandom.h>

static int num_iterations = 1000;
module_param(num_iterations, int, S_IRUGO); // 定义模块参数num_iterations，并设置为可读
MODULE_PARM_DESC(num_iterations, "Number of iterations for SMC call"); // 添加参数描述

#define ARM_STD_SVC_COUNTERS		0x8400ff04

static inline bool smccc_std_svc_counters(uint64_t *ctr)
{
	struct arm_smccc_res res;

	arm_smccc_1_1_invoke(ARM_STD_SVC_COUNTERS, (struct arm_smccc_res *)&res);
	if ((int)res.a0 >= 0) {
		*ctr = res.a3;
		return true;
	} else {
		return false;
	}
}

static int smc_ctr_show(struct seq_file *m, void *v)
{
	uint64_t ctr;
	bool has_trng;

	// Test Standard Service Counter
	has_trng = smccc_std_svc_counters(&ctr);
	if (!has_trng) {
		seq_printf(m, "Standard Service Counter not available\n");
	} else {
		seq_printf(m, "%d\n", ctr);
	}

	return 0;
}

static int smc_ctr_open(struct inode *inode, struct file *file)
{
    return single_open(file, smc_ctr_show, NULL);
}

static const struct proc_ops smc_ctr_fops = {
    .proc_open = smc_ctr_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

static int __init smc_call_init(void)
{
	uint64_t ctr;
	bool has_trng;
	u64 start_time, stop_time, total_time;
	int i;

	proc_create("smc_ctr", 0, NULL, &smc_ctr_fops);

	// Test Standard Service Counter
	has_trng = smccc_std_svc_counters(&ctr);
	if (!has_trng) {
		printk(KERN_ERR "Standard Service Counter not available\n");
	}

	printk(KERN_INFO "Standard Service Counter: %ld\n", ctr);

	for (i = 0; i < num_iterations; i++) {
		start_time = ktime_get();
		has_trng = smccc_std_svc_counters(&ctr);
		stop_time = ktime_get();

		total_time += ktime_sub(stop_time, start_time);
	}

	printk(KERN_INFO "Average SMC call time: %ld nanoseconds\n", total_time / num_iterations);

	return 0;
}

static void __exit smc_call_exit(void)
{
    remove_proc_entry("smc_ctr", NULL);

	printk(KERN_INFO "SMC call module exit\n");
}

module_init(smc_call_init);
module_exit(smc_call_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Author");
MODULE_DESCRIPTION("A simple example to make SMC call from Linux");

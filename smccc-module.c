#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/arm-smccc.h>
#include <linux/time.h>
#include <linux/ktime.h>
#include <asm/archrandom.h>

static int num_iterations = 1000;
module_param(num_iterations, int, S_IRUGO); // 定义模块参数num_iterations，并设置为可读
MODULE_PARM_DESC(num_iterations, "Number of iterations for SMC call"); // 添加参数描述

#if 1
#define ARM_STD_SVC_COUNTERS		0x8400ff04

static inline bool smccc_std_svc_counters(uint64_t *ctr)
{
	struct arm_smccc_res res;

	arm_smccc_1_1_invoke(ARM_STD_SVC_COUNTERS, (struct arm_smccc_res *)&res);
	printk(KERN_INFO "SMC Counter: %ld, %ld, %ld, %ld\n", res.a0, res.a1, res.a2, res.a3);
	if ((int)res.a0 >= 0) {
		*ctr = res.a3;
		return true;
	} else {
		return false;
	}
}
#else
#define ARM_SMCCC_TRNG_SMC_CTR				\
	ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL,			\
			   ARM_SMCCC_SMC_64,			\
			   ARM_SMCCC_OWNER_STANDARD,		\
			   0x54)

static inline bool smccc_trng_smc_ctr(uint64_t *ctr)
{
	struct arm_smccc_res res;

	arm_smccc_1_1_invoke(ARM_SMCCC_TRNG_SMC_CTR, (struct arm_smccc_res *)&res);
	printk(KERN_INFO "SMC Counter: %ld, %ld, %ld, %ld\n", res.a0, res.a1, res.a2, res.a3);
	if ((int)res.a0 >= 0) {
		*ctr = res.a3;
		return true;
	} else {
		return false;
	}
}
#endif

static int __init smc_call_init(void)
{
	uint64_t ctr;
	bool has_trng;
	u64 start_time, stop_time, total_time;
	int i;

#if 1
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
#else
	// Test SMC Counter
	has_trng = smccc_trng_smc_ctr(&ctr);
	if (!has_trng) {
		printk(KERN_ERR "SMC Counter not available\n");
	}

	printk(KERN_INFO "SMC Counter: %ld\n", ctr);
	has_trng = smccc_trng_smc_ctr(&ctr);
	if (!has_trng) {
		printk(KERN_ERR "SMC Counter not available\n");
	}

	printk(KERN_INFO "SMC Counter: %ld\n", ctr);
	printk(KERN_INFO "SMC call module init\n");
	has_trng = smccc_probe_trng();
	if (!has_trng) {
		printk(KERN_ERR "TRNG not available\n");
		return -ENODEV;
	}

	for (i = 0; i < num_iterations; i++) {
		start_time = ktime_get();
		has_trng = smccc_probe_trng();
		stop_time = ktime_get();

		total_time += ktime_sub(stop_time, start_time);
	}
#endif

	printk(KERN_INFO "Average SMC call time: %ld nanoseconds\n", total_time / num_iterations);

	return 0;
}

static void __exit smc_call_exit(void)
{
	printk(KERN_INFO "SMC call module exit\n");
}

module_init(smc_call_init);
module_exit(smc_call_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Author");
MODULE_DESCRIPTION("A simple example to make SMC call from Linux");

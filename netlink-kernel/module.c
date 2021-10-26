#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("zouxiaoliang");

extern int kmodule_init(void);
extern void kmodule_exit(void);

/**
 * @brief main_init
 * @return
 */
int main_init(void) { return kmodule_init(); }

/**
 * @brief main_exit
 */
void main_exit(void) { kmodule_exit(); }

module_init(main_init);
module_exit(main_exit);

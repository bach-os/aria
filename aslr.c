#include <aria/compiler.h>
#include <aria/debug.h>
#include <aria/address.h>
#include <aria/slab.h>
#include <aria/string.h>
#include <aria/aslr.h>

int aslr_generate_layout(struct aslr *aslr, struct aslr_layout **ret,
						 size_t length)
{
	if (unlikely(aslr == NULL || ret == NULL))
		return -1;

	struct aslr_layout *layout = alloc(sizeof(struct aslr_layout));
	if (unlikely(layout == NULL))
		return -1;

	*ret = layout;

	for (;;) {
		layout->lower_bound =
			ALIGN_UP(aslr->minimum_vaddr +
						 (({
							  uint64_t ret;
							  __asm__ volatile("rdrand %0" : "=r"(ret));
							  ret;
						  }) %
						  (aslr->maximum_vaddr - aslr->minimum_vaddr)),
					 PAGE_SIZE);
		layout->upper_bound = layout->lower_bound + length;

		for (struct aslr_layout *root = aslr->layout; root; root = root->next) {
			if (root->upper_bound <= layout->lower_bound ||
				layout->upper_bound <= root->lower_bound)
				continue;
			else
				goto next;
		}
		break;
next:
		continue;
	}

	layout->next = aslr->layout;
	layout->last = NULL;
	if (aslr->layout)
		aslr->layout->last = layout;
	aslr->layout = layout;

	return 0;
}

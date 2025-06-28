#include <aria/slab.h>
#include <aria/compiler.h>
#include <aria/string.h>
#include <aria/debug.h>
#include <aria/elf.h>
#include <aria/address.h>

static int elf64_validate(struct elf64_hdr *hdr)
{
	uint32_t signature = *(uint32_t *)hdr;
	if (signature != ELF_SIGNATURE)
		RETURN_ERROR;

	if (hdr->ident[ELF_EI_OSABI] != ELF_EI_SYSTEM_V &&
		hdr->ident[ELF_EI_OSABI] != ELF_EI_LINUX)
		RETURN_ERROR;
	if (hdr->ident[ELF_EI_DATA] != ELF_LITTLE_ENDIAN)
		RETURN_ERROR;
	if (hdr->ident[ELF_EI_CLASS] != ELF_ELF64)
		RETURN_ERROR;
	if (hdr->machine != ELF_MACH_X86_64 && hdr->machine != 0)
		RETURN_ERROR;

	return 0;
}

static int elf64_find_section(struct elf64_file *file, struct elf64_shdr **shdr,
							  const char *name)
{
	if (unlikely(file == NULL || shdr == NULL))
		RETURN_ERROR;

	for (int i = 0; i < file->hdr->sh_num; i++) {
		if (memcmp(&file->strtab[file->shdr[i].sh_name], name, strlen(name)) ==
			0) {
			*shdr = &file->shdr[i];
			return 0;
		}
	}

	*shdr = NULL;

	return 0;
}

#define VADDR_TO_OFFSET(ADDR)                                   \
	({                                                          \
		uintptr_t _addr = (ADDR);                               \
		if (_addr) {                                            \
			for (int _i = 0; _i < file->hdr->ph_num; _i++) {    \
				struct elf64_phdr *phdr = &file->phdr[_i];      \
				if (phdr->p_vaddr <= _addr &&                   \
					(phdr->p_vaddr + phdr->p_filesz) > _addr) { \
					_addr -= phdr->p_vaddr;                     \
					_addr += phdr->p_offset;                    \
					break;                                      \
				}                                               \
			}                                                   \
		}                                                       \
		_addr;                                                  \
	})

int elf64_file_init(struct elf64_file *file)
{
	file->hdr = alloc(sizeof(struct elf64_hdr));
	if (file->hdr == NULL)
		RETURN_ERROR;

	int ret = file->elf64_read(file, file->hdr, 0, sizeof(struct elf64_hdr));
	if (unlikely(ret != sizeof(struct elf64_hdr)))
		RETURN_ERROR;

	file->phdr = alloc(sizeof(struct elf64_phdr) * file->hdr->ph_num);
	if (file->phdr == NULL)
		RETURN_ERROR;
	file->shdr = alloc(sizeof(struct elf64_shdr) * file->hdr->sh_num);
	if (file->shdr == NULL)
		RETURN_ERROR;

	ret = file->elf64_read(file, file->shdr, file->hdr->shoff,
						   sizeof(struct elf64_shdr) * file->hdr->sh_num);
	if (unlikely(ret !=
				 (int)sizeof(struct elf64_shdr) * (int)file->hdr->sh_num))
		RETURN_ERROR;

	ret = elf64_validate(file->hdr);
	if (unlikely(ret == -1))
		RETURN_ERROR;

	ret = file->elf64_read(file, file->phdr, file->hdr->phoff,
						   sizeof(struct elf64_phdr) * file->hdr->ph_num);
	if (unlikely(ret != (int)sizeof(struct elf64_phdr) * file->hdr->ph_num))
		RETURN_ERROR;

	file->strtab_hdr = &file->shdr[file->hdr->shstrndx];
	file->strtab = alloc(file->strtab_hdr->sh_size);
	if (file->strtab == NULL)
		RETURN_ERROR;

	ret = file->elf64_read(file, (char *)file->strtab,
						   file->strtab_hdr->sh_offset,
						   file->strtab_hdr->sh_size);
	if (unlikely(ret != (int)file->strtab_hdr->sh_size))
		RETURN_ERROR;

	uintptr_t minimum_vaddr = 0;
	uintptr_t maximum_vaddr = 0;

	for (int i = 0; i < file->hdr->ph_num; i++) {
		if (file->phdr[i].p_type != ELF_PT_LOAD)
			continue;
		if (file->phdr[i].p_vaddr < minimum_vaddr)
			minimum_vaddr = file->phdr[i].p_vaddr;
		if (file->phdr[i].p_vaddr + file->phdr[i].p_memsz > maximum_vaddr)
			maximum_vaddr = file->phdr[i].p_vaddr + file->phdr[i].p_memsz;
	}

	ret = aslr_generate_layout(file->aslr, &file->aslr_layout,
							   maximum_vaddr - minimum_vaddr);
	if (ret == -1)
		RETURN_ERROR;

	for (size_t i = 0; i < file->hdr->ph_num; i++) {
		if (file->phdr[i].p_type != ELF_PT_DYNAMIC)
			continue;
		struct elf64_phdr *phdr = &file->phdr[i];

		for (size_t j = 0; j < phdr->p_filesz / sizeof(struct elf64_dyn); j++) {
			struct elf64_dyn dyn;
			ret = file->elf64_read(
				file, &dyn, phdr->p_offset + j * sizeof(struct elf64_dyn),
				sizeof(struct elf64_dyn));
			if (ret == -1)
				RETURN_ERROR;

			switch (dyn.d_tag) {
			case ELF_DT_NULL:
				break;
			case ELF_DT_NEEDED:
				RETURN_ERROR;
			case ELF_DT_PLTRELSZ:
				file->dynamic.pltrel_size = dyn.d_un;
				break;
			case ELF_DT_PLTGOT:
				break;
			case ELF_DT_HASH:
				break;
			case ELF_DT_SYMTAB:
				file->dynamic.symtab_offset = VADDR_TO_OFFSET(dyn.d_un);
				break;
			case ELF_DT_RELA:
				file->dynamic.rela_offset = VADDR_TO_OFFSET(dyn.d_un);
				break;
			case ELF_DT_RELASZ:
				file->dynamic.rela_size = dyn.d_un;
				break;
			case ELF_DT_RELAENT:
				file->dynamic.rela_ent = dyn.d_un;
				break;
			case ELF_DT_STRSZ:
				break;
			case ELF_DT_SYMENT:
				file->dynamic.symtab_ent = dyn.d_un;
				break;
			case ELF_DT_INIT:
				break;
			case ELF_DT_FINI:
				break;
			case ELF_DT_SONAME:
				break;
			case ELF_DT_RPATH:
				break;
			case ELF_DT_SYMBOLIC:
				break;
			case ELF_DT_RELR:
				file->dynamic.rel_offset = VADDR_TO_OFFSET(dyn.d_un);
				break;
			case ELF_DT_RELRSZ:
				file->dynamic.rel_size = dyn.d_un;
				break;
			case ELF_DT_RELRENT:
				file->dynamic.rel_ent = dyn.d_un;
				break;
			case ELF_DT_PLTREL:
				file->dynamic.pltrel = dyn.d_un;
				break;
			case ELF_DT_DEBUG:
				break;
			case ELF_DT_TEXTREL:
				break;
			case ELF_DT_JMPREL:
				file->dynamic.jmprel = VADDR_TO_OFFSET(dyn.d_un);
				break;
			case ELF_DT_BIND_NOW:
				break;
			case ELF_DT_RUNPATH:
				break;
			case ELF_DT_LOPROC:
			case ELF_DT_HIPROC:
				break;
			default:
				break;
			}
		}
	}

	return 0;
}

static int elf64_resolve_relocation(struct elf64_file *file,
									struct elf64_rela *relocation,
									uintptr_t vaddr, size_t section_offset)
{
	if (file == NULL || relocation == NULL)
		RETURN_ERROR;

	int offset = section_offset - vaddr + relocation->r_offset;

	uint64_t data;
	int ret = file->elf64_read(file, &data, offset, sizeof(data));
	if (ret == -1)
		RETURN_ERROR;

	switch (relocation->r_info) {
	case R_X86_64_NONE:
		break;
	case R_X86_64_64: {
		struct elf64_symtab symtab;
		int ret = file->elf64_read(file, &symtab,
								   file->dynamic.symtab_offset +
									   file->dynamic.symtab_ent *
										   (relocation->r_info >> 32),
								   sizeof(struct elf64_symtab));
		if (ret == -1)
			RETURN_ERROR;

		if (symtab.st_shndx == ELF_SHN_UNDEF &&
			(symtab.st_info >> 4) == ELF_STB_WEAK)
			data = 0;
		else
			data = file->aslr_layout->lower_bound + symtab.st_value +
				   relocation->r_addend;

		break;
	}
	case R_X86_64_PC32:
		RETURN_ERROR;
	case R_X86_64_GOT32:
		RETURN_ERROR;
	case R_X86_64_PLT32:
		RETURN_ERROR;
	case R_X86_64_COPY:
		RETURN_ERROR;
	case R_X86_64_GLOB_DAT:
	case R_X86_64_JMP_SLOT: {
		struct elf64_symtab symtab;
		int ret = file->elf64_read(file, &symtab,
								   file->dynamic.symtab_offset +
									   file->dynamic.symtab_ent *
										   (relocation->r_info >> 32),
								   sizeof(struct elf64_symtab));
		if (ret == -1)
			RETURN_ERROR;

		if (symtab.st_shndx == ELF_SHN_UNDEF &&
			(symtab.st_info >> 4) == ELF_STB_WEAK)
			data = 0;
		else
			data = file->aslr_layout->lower_bound + symtab.st_value;

		break;
	}
	case R_X86_64_RELATIVE:
		data = file->aslr_layout->lower_bound + relocation->r_addend;
		break;
	case R_X86_64_GOTPCREL:
		break;
	case R_INTERNAL_REL:
		data = data + file->aslr_layout->lower_bound;
		break;
	default:
		RETURN_ERROR;
	}

	ret = file->elf64_write(file, &data, offset, sizeof(data));
	if (ret == -1)
		RETURN_ERROR;

	return 0;
}

static int elf64_apply_relocation(struct elf64_file *file, uintptr_t vaddr,
								  size_t length, size_t section_offset)
{
	for (size_t offset = 0; offset < file->dynamic.pltrel_size;
		 offset += file->dynamic.rela_ent) {
		struct elf64_rela relocation;
		int ret = file->elf64_read(file, &relocation,
								   file->dynamic.jmprel + offset,
								   sizeof(struct elf64_rela));
		if (ret == -1)
			RETURN_ERROR;

		if (relocation.r_offset < vaddr ||
			(vaddr + length) < (relocation.r_offset + 8))
			continue;

		ret =
			elf64_resolve_relocation(file, &relocation, vaddr, section_offset);
		if (ret == -1)
			RETURN_ERROR;
	}

	for (size_t offset = 0; offset < file->dynamic.rela_size;
		 offset += file->dynamic.rela_ent) {
		struct elf64_rela relocation;
		int ret = file->elf64_read(file, &relocation,
								   file->dynamic.rela_offset + offset,
								   sizeof(struct elf64_rela));
		if (ret == -1)
			RETURN_ERROR;

		if (relocation.r_offset < vaddr ||
			(vaddr + length) < (relocation.r_offset + 8))
			continue;

		ret =
			elf64_resolve_relocation(file, &relocation, vaddr, section_offset);
		if (ret == -1)
			RETURN_ERROR;
	}

	// blob stolen from limine who stole it from https://maskray.me/blog/2021-10-31-relative-relocations-and-relr
	if (file->dynamic.rel_ent == 0) {
		return 0;
	}
	for (size_t i = 0; i < file->dynamic.rel_size / file->dynamic.rel_ent;
		 i++) {
		struct elf64_rela relocation = {
			.r_offset = ({
				uint64_t offset = 0, where = 0;
				uint64_t entry;
				int ret = file->elf64_read(file, &entry,
										   file->dynamic.rel_offset +
											   i * file->dynamic.rel_ent,
										   sizeof(entry));
				if (ret == -1)
					RETURN_ERROR;

				if ((entry & 1) == 0) {
					where = entry;
					offset = where;
					where += 8;
				} else {
					for (size_t j = 0; (entry >>= 1) != 0; j++) {
						if ((entry & 1) != 0) {
							offset = where + j * 8;
						}
					}
					where += 63 * 8;
				}
				offset;
			}),
			.r_info = R_INTERNAL_REL
		};

		if ((relocation.r_offset < vaddr) ||
			(vaddr + length) < (relocation.r_offset + 8))
			continue;

		int ret =
			elf64_resolve_relocation(file, &relocation, vaddr, section_offset);
		if (ret == -1)
			RETURN_ERROR;
	}

	return 0;
}

int elf64_load_section(struct elf64_file *file, const char *name, bool place)
{
	struct elf64_shdr *shdr;
	int ret = elf64_find_section(file, &shdr, name);
	if (ret == -1)
		RETURN_ERROR;
	if (shdr == NULL)
		return -1;

	ret = elf64_apply_relocation(file, shdr->sh_addr, shdr->sh_size,
								 shdr->sh_offset);
	if (ret == -1)
		RETURN_ERROR;

	if (unlikely(file->elf64_map == NULL))
		RETURN_ERROR;
	ret = file->elf64_map(file, NULL, shdr, place);
	if (ret == -1)
		return -1;

	return 0;
}

int elf64_file_load(struct elf64_file *file)
{
	int ret = elf64_load_section(file, ".bss", false);
	if (ret == -1)
		RETURN_ERROR;

	for (size_t i = 0; i < file->hdr->ph_num; i++) {
		if (file->phdr[i].p_type != ELF_PT_LOAD)
			continue;
		struct elf64_phdr *phdr = &file->phdr[i];

		int ret = elf64_apply_relocation(file, phdr->p_vaddr, phdr->p_memsz,
										 phdr->p_offset);
		if (ret == -1)
			RETURN_ERROR;

		if (unlikely(file->elf64_map == NULL))
			RETURN_ERROR;
		ret = file->elf64_map(file, phdr, NULL, true);
		if (ret == -1)
			return -1;
	}

	return 0;
}

int elf64_file_aux(struct elf64_file *file, struct aux *aux)
{
	aux->at_phdr = 0;
	aux->at_phent = sizeof(struct elf64_phdr);
	aux->at_phnum = file->hdr->ph_num;
	aux->at_entry = file->hdr->entry + file->aslr_layout->lower_bound;

	for (size_t i = 0; i < file->hdr->ph_num; i++) {
		if (file->phdr[i].p_type == ELF_PT_PHDR) {
			aux->at_phdr =
				file->phdr[i].p_vaddr + file->aslr_layout->lower_bound;
		}
	}

	return 0;
}

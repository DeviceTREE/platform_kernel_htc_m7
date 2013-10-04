/*
 *  arch/arm/include/asm/checksum.h
 *
 * IP checksum routines
 *
 * Copyright (C) Original authors of ../asm-i386/checksum.h
 * Copyright (C) 1996-1999 Russell King
 */
#ifndef __ASM_ARM_CHECKSUM_H
#define __ASM_ARM_CHECKSUM_H

#include <linux/in6.h>

__wsum csum_partial(const void *buff, int len, __wsum sum);


__wsum
csum_partial_copy_nocheck(const void *src, void *dst, int len, __wsum sum);

__wsum
csum_partial_copy_from_user(const void __user *src, void *dst, int len, __wsum sum, int *err_ptr);

static inline __sum16 csum_fold(__wsum sum)
{
	__asm__(
	"add	%0, %1, %1, ror #16	@ csum_fold"
	: "=r" (sum)
	: "r" (sum)
	: "cc");
	return (__force __sum16)(~(__force u32)sum >> 16);
}

static inline __sum16
ip_fast_csum(const void *iph, unsigned int ihl)
{
	unsigned int tmp1;
	__wsum sum;

	__asm__ __volatile__(
	"ldr	%0, [%1], #4		@ ip_fast_csum		\n\
	ldr	%3, [%1], #4					\n\
	sub	%2, %2, #5					\n\
	adds	%0, %0, %3					\n\
	ldr	%3, [%1], #4					\n\
	adcs	%0, %0, %3					\n\
	ldr	%3, [%1], #4					\n\
1:	adcs	%0, %0, %3					\n\
	ldr	%3, [%1], #4					\n\
	tst	%2, #15			@ do this carefully	\n\
	subne	%2, %2, #1		@ without destroying	\n\
	bne	1b			@ the carry flag	\n\
	adcs	%0, %0, %3					\n\
	adc	%0, %0, #0"
	: "=r" (sum), "=r" (iph), "=r" (ihl), "=r" (tmp1)
	: "1" (iph), "2" (ihl)
	: "cc", "memory");
	return csum_fold(sum);
}

static inline __wsum
csum_tcpudp_nofold(__be32 saddr, __be32 daddr, unsigned short len,
		   unsigned short proto, __wsum sum)
{
	__asm__(
	"adds	%0, %1, %2		@ csum_tcpudp_nofold	\n\
	adcs	%0, %0, %3					\n"
#ifdef __ARMEB__
	"adcs	%0, %0, %4					\n"
#else
	"adcs	%0, %0, %4, lsl #8				\n"
#endif
	"adcs	%0, %0, %5					\n\
	adc	%0, %0, #0"
	: "=&r"(sum)
	: "r" (sum), "r" (daddr), "r" (saddr), "r" (len), "Ir" (htons(proto))
	: "cc");
	return sum;
}	
static inline __sum16
csum_tcpudp_magic(__be32 saddr, __be32 daddr, unsigned short len,
		  unsigned short proto, __wsum sum)
{
	return csum_fold(csum_tcpudp_nofold(saddr, daddr, len, proto, sum));
}


static inline __sum16
ip_compute_csum(const void *buff, int len)
{
	return csum_fold(csum_partial(buff, len, 0));
}

#define _HAVE_ARCH_IPV6_CSUM
extern __wsum
__csum_ipv6_magic(const struct in6_addr *saddr, const struct in6_addr *daddr, __be32 len,
		__be32 proto, __wsum sum);

static inline __sum16
csum_ipv6_magic(const struct in6_addr *saddr, const struct in6_addr *daddr, __u32 len,
		unsigned short proto, __wsum sum)
{
	return csum_fold(__csum_ipv6_magic(saddr, daddr, htonl(len),
					   htonl(proto), sum));
}
#endif
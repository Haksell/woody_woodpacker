/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strcpy.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amyroshn <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/01/24 09:02:39 by amyroshn          #+#    #+#             */
/*   Updated: 2022/01/24 09:02:42 by amyroshn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char *ft_strdup_func(const char* src, int (*f)(int)) {
    char    *dest;
    size_t  i;

    dest = malloc((ft_strlen(src) + 1) * sizeof(char));
    if (dest == NULL) {
        return NULL;
    }
    for (i = 0; src[i]; i++) {
        dest[i] = f(src[i]);
    }
    dest[i] = '\0';
    return dest;
}
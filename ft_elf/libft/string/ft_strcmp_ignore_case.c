/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strcmp.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: amyroshn <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/01/24 09:03:24 by amyroshn          #+#    #+#             */
/*   Updated: 2022/01/24 09:03:27 by amyroshn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "libft.h"

int	ft_strcmp_ignore_case(const char *s1, const char *s2)
{
    int	i;

    i = 0;
    if (!s1 && !s2)
        return (0);
    if (!s1 || !s2)
        return (1);
    while (s1[i] || s2[i])
    {
        int s1_c = ft_tolower(s1[i]);
        int s2_c = ft_tolower(s2[i]);
        if (s1_c != s2_c)
            return (s1_c - s2_c);
        i++;
    }
    return (0);
}

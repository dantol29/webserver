# include <unistd.h>
# include <stdlib.h>
#include <cstring>
#ifndef BUFFER_SIZE
# define BUFFER_SIZE 42
#endif

char	*ft_strchr(char *s, int c)
{
	while (*s)
	{
		if (*s == (char)c)
			return ((char *)s);
		s++;
	}
	return (NULL);
}

size_t	ft_strlen(const char *s)
{
	size_t	i = 0;
	
	while (s[i])
		i++;
	return (i);
}


char	*ft_strdup(const char *src)
{
	size_t	len = ft_strlen(src) + 1;
	char	*dst = (char *)malloc(len);
	
	if (dst == NULL)
		return (NULL);
	std::strcpy(dst, src);
	return (dst);
}

char	*ft_strjoin(char *s1, char const *s2)
{
	size_t	s1_len = ft_strlen(s1);
	size_t	s2_len = ft_strlen(s2);
	char	*join = (char *)malloc((s1_len + s2_len + 1));

	if (!s1 || !s2)
		return (NULL);
	if (!join)
		return (NULL);
	std::strcpy(join, s1);
	std::strcpy((join + s1_len), s2);
	free(s1);
	return (join);
}

char	*get_next_line(int fd)
{
	static char	buf[BUFFER_SIZE + 1];
	char		*line;
	char		*newline;
	int			countread;
	int			to_copy;

	line = ft_strdup(buf);
	while (!(newline = ft_strchr(line, '\n')) && (countread = read(fd, buf, BUFFER_SIZE)))
	{
		buf[countread] = '\0';
		line = ft_strjoin(line, buf);
	}
	if (ft_strlen(line) == 0){
		free(line);
		return (NULL);
	}

	if (newline != NULL)
	{
		to_copy = newline - line + 1;
		std::strcpy(buf, newline + 1);
	}
	else
	{
		to_copy = ft_strlen(line);
		buf[0] = '\0';
	}
	line[to_copy] = '\0';
	return (line);
}
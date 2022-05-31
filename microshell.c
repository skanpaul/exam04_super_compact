/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   microshell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sorakann <sorakann@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/05/30 16:44:07 by ski               #+#    #+#             */
/*   Updated: 2022/05/31 07:13:11 by sorakann         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

# define TYPE_NORMAL	0
# define TYPE_PIPE		1

/* ************************************************************************** */
typedef struct	s_data
{
	int status;
	int	output_type;
	int	stdin_original;
}				t_data;

/* ************************************************************************** */
int		ft_strlen(char *str);
void	ft_cd(char **argv);
void	ft_exec(t_data *data, char **argv, char **envp);

/* ************************************************************************** */
int	main(int argc, char **argv, char **envp)
{
	(void)argc;
	int i = 1;
	int j = 1;
	t_data data;
	data.status = 0;
	data.output_type = TYPE_NORMAL;
	data.stdin_original = dup(STDIN_FILENO);

	while (argv[i])
	{
		// ---------------------------------------------------------
		if (strcmp(argv[i], "|") == 0 || strcmp(argv[i], ";") == 0)
		{
			if (strcmp(argv[i], "|") == 0)
				data.output_type = 1;
			argv[i] = NULL;
			ft_exec(&data, &argv[j], envp);
			data.output_type = TYPE_NORMAL;
			j = i + 1;
		}
		// ---------------------------------------------------------
		else if (argv[i + 1] == NULL)
			ft_exec(&data, &argv[j], envp);
		// ---------------------------------------------------------
		i++;
		// ---------------------------------------------------------
	}
	close(data.stdin_original);
	return (0);
}

/* ************************************************************************** */

int	ft_strlen(char *str)
{
	int i = 0;

	while (str[i])
		i++;
	return (i);
}

/* ************************************************************************** */
void	ft_cd(char **argv)
{
	int i = 0;
	char *arg_error = "error: cd: bad arguments\n";
	char *path_error = "error: cd: cannot change directory to ";

	while (argv[i])
		i++;
	if (i != 2)
		write(2, arg_error, ft_strlen(arg_error));
	else if (chdir(argv[1]) == -1)
	{
		write(2, path_error, ft_strlen(path_error));
		write(2, argv[1], ft_strlen(argv[1]));
		write(2, "\n", 1);
	}
}

/* ************************************************************************** */
void	ft_exec(t_data *data, char **argv, char **envp)
{
	int fd;
	int	pipefd[2];
	char *fatal_error = "error: fatal\n";
	char *cmd_error = "error: cannot execute ";

	
	// ---------------------------------------------------------
	// ---------------------------------------------------------
	if (argv[0] == NULL)
		return ;
	// ---------------------------------------------------------
	// ---------------------------------------------------------
	if (strcmp(argv[0], "cd") == 0)
	{
		ft_cd(argv);
		return ;
	}
	// ---------------------------------------------------------
	// ---------------------------------------------------------
	if (data->output_type == TYPE_PIPE && pipe(pipefd) == -1)
	{
		write(2, fatal_error, ft_strlen(fatal_error));
		exit(1);
	}
	// ---------------------------------------------- fork_error
	fd = fork();
	if (fd == -1)
	{
		write(2, fatal_error, ft_strlen(fatal_error));
		exit(1);
	}
	// ---------------------------------------------- fork_CHILD
	if (fd == 0)
	{
		if (data->output_type == TYPE_PIPE)
		{
			close(pipefd[0]);
			dup2(pipefd[1], STDOUT_FILENO);
			close(pipefd[1]);

		}
		execve(argv[0], argv, envp);
		write(2, cmd_error, ft_strlen(cmd_error));
		write(2, argv[0], ft_strlen(argv[0]));
		write(2, "\n", 1);
		close(data->stdin_original);
		exit(1);
	}
	// --------------------------------------------- fork_PARENT
	else
	{
		if (data->output_type == TYPE_PIPE)
		{
			close(pipefd[1]);
			dup2(pipefd[0], STDIN_FILENO);
			close(pipefd[0]);
		}
		else
			dup2(data->stdin_original, STDIN_FILENO);
		waitpid(fd, &data->status, 0);
	}
	// ---------------------------------------------------------
}

/* ************************************************************************** */
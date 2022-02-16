#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>


#include <stdio.h>


typedef struct	s_data
{
	int status;
	int	stdout_pipe;
	int	old_stdin;
}				t_data;

int	ft_strlen(char *str)
{
	int i = 0;

	while (str[i])
		i++;
	return (i);
}

void	ft_cd(char **av)
{
	int i = 0;
	char *arg_error = "error: cd: bad arguments\n";
	char *path_error = "error: cd: cannot change directory to ";

	while (av[i])
		i++;
	if (i != 2)
		write(2, arg_error, ft_strlen(arg_error));
	else if (chdir(av[1]) == -1)
	{
		write(2, path_error, ft_strlen(path_error));
		write(2, av[1], ft_strlen(av[1]));
		write(2, "\n", 1);
	}
}

void	ft_exec(t_data *data, char **av, char **envp)
{
	int fd;
	int	pipefd[2];
	char *fatal_error = "error: fatal\n";
	char *cmd_error = "error: cannot execute ";

	if (av[0] == NULL)
		return ;
	if (strcmp(av[0], "cd") == 0)
	{
		ft_cd(av);
		return ;
	}
	if (data->stdout_pipe == 1 && pipe(pipefd) == -1)
	{
		write(2, fatal_error, ft_strlen(fatal_error));
		exit(1);
	}
	fd = fork();
	if (fd == -1)
	{
		write(2, fatal_error, ft_strlen(fatal_error));
		exit(1);
	}
	if (fd == 0)
	{
		if (data->stdout_pipe == 1)
		{
			close(pipefd[0]);
			dup2(pipefd[1], STDOUT_FILENO);
			close(pipefd[1]);
		}
		execve(av[0], av, envp);
		write(2, cmd_error, ft_strlen(cmd_error));
		write(2, av[0], ft_strlen(av[0]));
		write(2, "\n", 1);
		close(data->old_stdin);
		exit(1);
	}
	else
	{
		if (data->stdout_pipe == 1)
		{
			close(pipefd[1]);
			dup2(pipefd[0], STDIN_FILENO);
			close(pipefd[0]);
		}
		else
			dup2(data->old_stdin, STDIN_FILENO);
		waitpid(fd, &data->status, 0);
	}
}

int	main(int ac, char **av, char **envp)
{
	(void)ac;
	int i = 1;
	int j = 1;
	t_data data;
	data.status = 0;
	data.stdout_pipe = 0;
	data.old_stdin = dup(STDIN_FILENO);

	while (av[i])
	{
		if (strcmp(av[i], "|") == 0 || strcmp(av[i], ";") == 0)
		{
			if (strcmp(av[i], "|") == 0)
				data.stdout_pipe = 1;
			av[i] = NULL;
			ft_exec(&data, &av[j], envp);
			data.stdout_pipe = 0;
			j = i + 1;
		}
		else if (av[i + 1] == NULL)
			ft_exec(&data, &av[j], envp);
		i++;
	}
	close(data.old_stdin);
	return (0);
}

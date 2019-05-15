<style>body{font-family:Times New Roman;font-size:13px;}td{font-family:Monaco;font-size:11px;}</style>

<div align="center" style="font-size:24px;"><b>Hand-written Assignment 5</b></div>

<div align="center" style="font-size:16px;"><i>Student ID</i>: B03902048 <i>Name</i>: 林義聖</div>

<hr>

### Form

<table>
<tr>
	<th></th>
	<th>Model 1</th>
	<th>Model 2</th>
</tr>
<tr>
	<th>Section A</th>
	<td>
	int n, pfd[3][2];<br>
    char buf[1024];<br>
    pipe(pfd[0]);<br>
    pipe(pfd[1]);<br>
    pipe(pfd[2]);
    </td>
	<td>
	int n, pfd[2];<br>
    char buf[1024];<br>
    pipe(pfd);
	</td>
</tr>
<tr>
	<th>Section B</th>
	<td></td>
	<td></td>
</tr>
<tr>
	<th>Section C</th>
	<td>
	dup2(pfd[i-1][1], STDOUT_FILENO);<br>
    close(pfd[0][0]);<br>
    close(pfd[0][1]);<br>
    close(pfd[1][0]);<br>
    close(pfd[1][1]);<br>
    close(pfd[2][0]);<br>
    close(pfd[2][1]);
	</td>
	<td>
	dup2(pfd[1], STDOUT_FILENO);<br>
    close(pfd[0]);<br>
    close(pfd[1]);
	</td>
</tr>
<tr>
	<th>Section D</th>
	<td>close(pfd[i-1][1]);</td>
	<td></td>
</tr>
<tr>
	<th>Section E</th>
	<td>
	while (wait(NULL) > 0);<br>
    for (i=0; i<3; i++) {<br>
        while ((n = read(pfd[i][0], buf, 1024)) > 0) {<br>
            write(STDOUT_FILENO, buf, n);<br>
        }<br>
        close(pfd[i][0]);<br>
    }
	</td>
	<td>
	close(pfd[1]);<br>
    while (wait(NULL) > 0);<br>
    while ((n = read(pfd[0], buf, 1024)) > 0) {<br>
        write(STDOUT_FILENO, buf, n);<br>
    }<br>
    close(pfd[0]);
	</td>
</tr>
</table>

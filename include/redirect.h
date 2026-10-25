#pragma once
void redirect(int prev_fd, int new_fd);
void redirect_stdin(const char* infile);
void redirect_stdout(const char* outfile);
void extract_redirect(char* args[], char** infile, char** outfile);
int open_infile(const char *infile);
int open_outfile(const char *infile);
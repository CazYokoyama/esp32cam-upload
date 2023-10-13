// ESP32 libssh port.
//
// Ewan Parker, created 22nd April 2020.
// Simple port of examples/template.c over WiFi.  Run with a serial monitor at
// 115200 BAUD.
//
// Copyright (C) 2016–2022 Ewan Parker.

/* simple exec example */

// The command line you would use to run this from a shell prompt.
#define EX_CMD "exec"

// Stack size needed to run SSH and the command parser.
const unsigned int configSTACK = 51200;

#include "WiFi.h"
// Include the Arduino library.
#include "libssh_esp32.h"

// EXAMPLE includes/defines START
#include <libssh/libssh.h>
#include "examples_common.h"
// EXAMPLE includes/defines FINISH

#include "camera.h"

// EXAMPLE functions START
/*
 * knownhosts.c
 * This file contains an example of how verify the identity of a
 * SSH server using libssh
 */

/*
Copyright 2003-2009 Aris Adamantiadis

This file is part of the SSH Library

You are free to copy this file, modify it in any way, consider it being public
domain. This does not apply to the rest of the library though, but it is
allowed to cut-and-paste working code from this file to any license of
program.
The goal is to show the API in action. It's not a reference on how terminal
clients must be made or how a client should react.
 */

#include "libssh/priv.h"

int verify_knownhost(ssh_session session)
{
    enum ssh_known_hosts_e state;
    char buf[10];
    unsigned char *hash = NULL;
    size_t hlen;
    ssh_key srv_pubkey;
    int rc;

    rc = ssh_get_server_publickey(session, &srv_pubkey);
    if (rc < 0) {
        return -1;
    }

    rc = ssh_get_publickey_hash(srv_pubkey,
                                SSH_PUBLICKEY_HASH_SHA256,
                                &hash,
                                &hlen);
    ssh_key_free(srv_pubkey);
    if (rc < 0) {
        return -1;
    }

    state = ssh_session_is_known_server(session);

    switch(state) {
    case SSH_KNOWN_HOSTS_CHANGED:
        fprintf(stderr,"Host key for server changed : server's one is now :\n");
        ssh_print_hash(SSH_PUBLICKEY_HASH_SHA256, hash, hlen);
        ssh_clean_pubkey_hash(&hash);
        fprintf(stderr,"For security reason, connection will be stopped\n");
        return -1;
    case SSH_KNOWN_HOSTS_OTHER:
        fprintf(stderr,"The host key for this server was not found but an other type of key exists.\n");
        fprintf(stderr,"An attacker might change the default server key to confuse your client"
                "into thinking the key does not exist\n"
                "We advise you to rerun the client with -d or -r for more safety.\n");
        return -1;
    case SSH_KNOWN_HOSTS_NOT_FOUND:
        fprintf(stderr,"Could not find known host file. If you accept the host key here,\n");
        fprintf(stderr,"the file will be automatically created.\n");
        /* fallback to SSH_SERVER_NOT_KNOWN behavior */
        FALL_THROUGH;
    case SSH_SERVER_NOT_KNOWN:
        break;
    case SSH_KNOWN_HOSTS_ERROR:
        ssh_clean_pubkey_hash(&hash);
        fprintf(stderr,"%s",ssh_get_error(session));
        return -1;
    case SSH_KNOWN_HOSTS_OK:
        break; /* ok */
    }

    ssh_clean_pubkey_hash(&hash);

    return 0;
}

/*
 * authentication.c
 * This file contains an example of how to do an authentication to a
 * SSH server using libssh
 */

/*
Copyright 2003-2009 Aris Adamantiadis

This file is part of the SSH Library

You are free to copy this file, modify it in any way, consider it being public
domain. This does not apply to the rest of the library though, but it is
allowed to cut-and-paste working code from this file to any license of
program.
The goal is to show the API in action. It's not a reference on how terminal
clients must be made or how a client should react.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libssh/libssh.h>
#include "examples_common.h"

static void error(ssh_session session)
{
    fprintf(stderr,"Authentication failed: %s\n",ssh_get_error(session));
}

int authenticate_console(ssh_session session)
{
    int rc;
    int method;
    char *banner;

    // Try to authenticate
    rc = ssh_userauth_none(session, NULL);
    if (rc == SSH_AUTH_ERROR) {
        error(session);
        return rc;
    }

    method = ssh_userauth_list(session, NULL);
    if (method & SSH_AUTH_METHOD_GSSAPI_MIC) {
      rc = ssh_userauth_gssapi(session);
      if (rc == SSH_AUTH_ERROR) {
	error(session);
	return rc;
      } else if (rc == SSH_AUTH_SUCCESS)
	goto return_success;
    }

    // Try to authenticate with public key first
    if (method & SSH_AUTH_METHOD_PUBLICKEY) {
      rc = ssh_userauth_publickey_auto(session, NULL, NULL);
      if (rc == SSH_AUTH_ERROR) {
	error(session);
	return rc;
      }
    }

 return_success:
    banner = ssh_get_issue_banner(session);
    if (banner) {
        printf("%s\n",banner);
        SSH_STRING_FREE_CHAR(banner);
    }

    return rc;
}

/*
 * connect_ssh.c
 * This file contains an example of how to connect to a
 * SSH server using libssh
 */

/*
Copyright 2009 Aris Adamantiadis

This file is part of the SSH Library

You are free to copy this file, modify it in any way, consider it being public
domain. This does not apply to the rest of the library though, but it is
allowed to cut-and-paste working code from this file to any license of
program.
The goal is to show the API in action. It's not a reference on how terminal
clients must be made or how a client should react.
 */

#include <libssh/libssh.h>
#include "examples_common.h"
#include <stdio.h>

ssh_session connect_ssh(const char *host, const char *user,int verbosity){
  ssh_session session;
  int auth=0;

  session=ssh_new();
  if (session == NULL) {
    return NULL;
  }

  if(user != NULL){
    if (ssh_options_set(session, SSH_OPTIONS_USER, user) < 0) {
      ssh_free(session);
      return NULL;
    }
  }

  if (ssh_options_set(session, SSH_OPTIONS_HOST, host) < 0) {
    ssh_free(session);
    return NULL;
  }
  ssh_options_set(session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
  ssh_options_set(session, SSH_OPTIONS_SSH_DIR, "/spiffs");
  if(ssh_connect(session)){
    fprintf(stderr,"Connection failed : %s\n",ssh_get_error(session));
    ssh_disconnect(session);
    ssh_free(session);
    return NULL;
  }
  if(verify_knownhost(session)<0){
    ssh_disconnect(session);
    ssh_free(session);
    return NULL;
  }
  auth=authenticate_console(session);
  if(auth==SSH_AUTH_SUCCESS){
    return session;
  } else if(auth==SSH_AUTH_DENIED){
    fprintf(stderr,"Authentication failed\n");
  } else {
    fprintf(stderr,"Error while authenticating : %s\n",ssh_get_error(session));
  }
  ssh_disconnect(session);
  ssh_free(session);
  return NULL;
}
// EXAMPLE functions FINISH

// EXAMPLE main START
int ex_main(int argc, char **argv){
    ssh_session session;
    ssh_channel channel;
    char buffer[256];
    int rbytes, wbytes, total = 0;
    int rc;

    session = connect_ssh("10.0.0.103", "caz", 0);
    if (session == NULL) {
        ssh_finalize();
        return 1;
    }

    channel = ssh_channel_new(session);
    if (channel == NULL) {
        ssh_disconnect(session);
        ssh_free(session);
        ssh_finalize();
        return 1;
    }

    rc = ssh_channel_open_session(channel);
    if (rc < 0) {
        goto failed;
    }

    rc = ssh_channel_request_exec(channel, "ls");
    if (rc < 0) {
        goto failed;
    }

    rbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    if (rbytes <= 0) {
        goto failed;
    }

    do {
        wbytes = fwrite(buffer + total, 1, rbytes, stdout);
        if (wbytes <= 0) {
            goto failed;
        }

        total += wbytes;

        /* When it was not possible to write the whole buffer to stdout */
        if (wbytes < rbytes) {
            rbytes -= wbytes;
            continue;
        }

        rbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
        total = 0;
    } while (rbytes > 0);

    if (rbytes < 0) {
        goto failed;
    }

    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    ssh_disconnect(session);
    ssh_free(session);
    ssh_finalize();

    return 0;
failed:
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    ssh_disconnect(session);
    ssh_free(session);
    ssh_finalize();

    return 1;
}
// EXAMPLE main FINISH

void controlTask(void *pvParameter)
{
  while (1)
  {
        // Initialize the Arduino library.
        libssh_begin();

        // Call the EXAMPLE main code.
        {
          char *ex_argv[] = { EX_CMD, NULL };
          int ex_argc = sizeof ex_argv/sizeof ex_argv[0] - 1;
          printf("%% Execution in progress:");
          short a; for (a = 0; a < ex_argc; a++) printf(" %s", ex_argv[a]);
          printf("\n\n");
          int ex_rc = ex_main(ex_argc, ex_argv);
          printf("\n%% Execution completed: rc=%d\n", ex_rc);
        }
        while (1) vTaskDelay(60000 / portTICK_PERIOD_MS);
  }
}

void ssh_setup()
{
  // Stack size needs to be larger, so continue in a new task.
  xTaskCreatePinnedToCore(controlTask, "ctl", configSTACK, NULL,
    (tskIDLE_PRIORITY + 3), NULL, portNUM_PROCESSORS - 1);
}

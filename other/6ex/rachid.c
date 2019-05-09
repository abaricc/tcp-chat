while(verif==0)
    {
      char cmd[MESSAGE_MAXLEN], response[MESSAGE_MAXLEN];
      while(1) {
	if(receive_message(client[i].sock, cmd, sizeof(cmd)) < 0)
	  break; /* erreur: on deconnecte le client */
	if(eval_msg(i, cmd, response, sizeof(response)) < 0)
	  break; /* erreur: on deconnecte le client */
	if(write(client[i].sock, response, strlen(response)) < 0) {
	  perror("could not send message");
	  break; /* erreur: on deconnecte le client */
	}
      }
      break;
    }

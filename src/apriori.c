/**
 * @file apriori.c
 * @brief apriori algorithm
 * @author Sahil Gupta
 * @date Februrary 2012
 */

/* LICENSE
 * Copyright 2013 Sahil Gupta
 * Licensed under the Apache License, Version 2.0 (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "dbapriori.h"
#include "apriori.h"
#include "dbbasic.h"
#include "logging.h"

/**
 * @brief Create a new Itemset
 * @param itemset_num
 * @return Itemset pointer
 * @author SG
 */
static I *newI(int itemset_num)
{
	I *tmpi = (I *)malloc(sizeof(I));
	tmpi->number = itemset_num;
	tmpi->count = 0;
	tmpi->candidate = (C *)malloc(sizeof(C));
	tmpi->nexti = NULL;
	strcpy(tmpi->candidate->members, "");
	tmpi->candidate->nextc = NULL;
	return tmpi;
}

/**
 * @brief Returns required itemset indicated by itemset number
 * @param itemset_num
 * @return Itemset pointer
 * @author SG
 */
static I* get_itemset(int itemset_num)
{
	I *previ;
	for(previ = headi; previ != NULL && previ->number != itemset_num ;
	    previ = previ->nexti);
	return previ;
}

/**
 * @brief Returns new token separated by separator
 * @param source_str Source String
 * @param tokenno Index of required token from source string
 * @param separator Character separating tokens in source string
 * @return token
 * @author SG
 */
char *get_token(char *source_str ,int tokenno, char separator)
{
	char *token;
	int i, j, cur;

	j = 0;
	cur = 0;
	token = (char *)malloc(strlen(source_str) * sizeof(char));
	strcpy(token,"");

	if(source_str == NULL || strcmp(source_str, "") == 0 ) {
		return token;
	}


	for(i = 0 ; source_str[i] != '\0' && cur <= tokenno ; i++) {
		if(source_str[i] == separator) {
			cur++;
			i++;
		}
		if(tokenno == cur) {
			token[j++] = source_str[i];
		}
	}
	token[j] = '\0';

	return token;
}

/**
 * @brief Returns individual items from a token
 * @param token
 * @param item_iter Iterator for item
 * @return item
 * @author SG
 */
char *get_item(char *token,int *item_iter)
{
	int i;
	char *item = (char *)malloc((MAX_ITEM_LENGTH + 1) * sizeof(char));

	for(i = 0; token[*item_iter] != '\0'; i++, *item_iter += 1) {
		if((token[*item_iter] == CHAR_ITEMSET_SEP) ||
		   (token[*item_iter] == CHAR_ITEM_SEP)) {
			break;
		}
		item[i] = token[*item_iter];
	}

	item[i] = '\0';
	*item_iter += 1;

	return item;
}

/**
 * @brief Total number of items in token
 * @param token
 * @return Count of items
 * @author SG
 */
int get_no_of_items(char *token)
{
	int cnt=0;
	int i;

	for(i = 0; token[i] != '\0'; i++) {
		if(token[i] == CHAR_ITEMSET_SEP || token[i] == CHAR_ITEM_SEP) {
			cnt++;
		}
	}

	/* Add 1 for last item in token */
	return cnt + 1;
}

/**
 * @brief Check if item is present in itemset
 * @param itemset
 * @param item To be checked
 * @param count Number of items in itemset
 * @param sep Separator
 * @return 1 : Item present, 0 : Item not present
 * @author SG
 */
int check_item(char *itemset, char *item, int count, char sep)
{
	char *token;
	int i;

	for(i = 0; i < count; i++) {
		token = get_token(itemset, i, sep);
		if(strcmp(token, item) == 0) {
			free((char *)token);
			return 1;
		}
		free((char *) token);
	}
	return 0;
}

/**
 * @brief Check if string has memory remaining
 * @param str Sting to be analyzed
 * @param no_of_item Memory required to be free
 * @return 1 : Item present, 0 : Item not present
 * @author SG
 */
int isfull(char *str, unsigned int no_of_items)
{
	if(strlen(str) < (MAX_ITEMSET_LENGTH -
	                 ((MAX_ITEM_LENGTH + 1) * no_of_items) + 1)) {
		return 0;
	} else {
		return 1;
	}
}

/*-------------------------- GENEREATE CANDIDATE -----------------------------*/

/**
 * @brief Generate candidates
 * @param itemset_num
 * @return Count of candidates generated
 * @author SG
 */
static int generate_candidates(int itemset_num)
{
	I *tmpi;
	void *tmp;
	int i,j,k;

	/* if its the first itemset, candidates are all items to be analyzed */
	if(itemset_num == 1) {
		const char *item;

		tmp = get_user_tagged_files();
		while ((item = string_from_stmt(tmp)) != NULL) {
			if(headi == NULL){
				headi = newI(itemset_num);
				tmpi = headi;
			}
			if(isfull(tmpi->candidate->members, 1)) {
				/** @todo : Add Candidate link list */
				log_msg("Insufficient memory to generate "
				         "%d-candidate", itemset_num);
				finalize(tmp);
				return headi->count;
			}
			strcat(tmpi->candidate->members, item);
			strcat(tmpi->candidate->members, STR_ITEMSET_SEP);
			headi->count += 1;
		}
		if(headi != NULL){
			return headi->count;
		} else { /* No user tagged files */
			log_msg("No items to analyze");
			return 0;
		}

	} else if(itemset_num == 2) { /* For 2-candidate Itemset , L1 join L1 */
		tmpi = headi->nexti = newI(itemset_num);

		for(i = 0; i < (headi->count - 1); i++) {
			for(j = (i + 1); j < headi->count; j++) {

				if(isfull(tmpi->candidate->members, 2)) {
					/** @todo : Add Candidate link list */
					log_msg("Insufficient memory to "
					 "generate %d-candidate", itemset_num);
					return tmpi->count;
				}

				tmp = get_token(headi->candidate->members, i,
						CHAR_ITEMSET_SEP);
				strcat(tmpi->candidate->members, tmp);
				free((char *)tmp);
				strcat(tmpi->candidate->members, STR_ITEM_SEP);

				tmp = get_token(headi->candidate->members, j,
						CHAR_ITEMSET_SEP);
				strcat(tmpi->candidate->members, tmp);
				free((char *)tmp);
				strcat(tmpi->candidate->members,
				        STR_ITEMSET_SEP);
				tmpi->count += 1;
			}
		}
		return tmpi->count;

	} else {
		I *previ;
		char Ci_item[MAX_ITEMSET_LENGTH],Cj_item[MAX_ITEMSET_LENGTH];
		char *Ci,*Cj;
		int Ci_iter,Cj_iter;

		/* Access last itemset */
		previ = get_itemset(itemset_num - 1);

		/* Create new Itemset for itemset_num */
		tmpi = newI(itemset_num);
		previ->nexti = tmpi;

		/* for each itemset */
		for(i = 0; i < (previ->count - 1); i++) {
			for(j = (i + 1); j < previ->count; j++) {
				Ci_iter = 0;
				Cj_iter = 0;
				strcpy(Ci_item, "");
				strcpy(Cj_item, "");
				Ci = get_token(previ->candidate->members, i,
				               CHAR_ITEMSET_SEP);
				Cj = get_token(previ->candidate->members, j,
				               CHAR_ITEMSET_SEP);

				/* make string of the first n-2 tokens */
				for(k = 0; k < (itemset_num - 2); k++) {
					if(isfull(Ci_item , 1)) {
						log_msg("Insufficient memory to"
						        " compute %d-candidate",
						          itemset_num);
						free((char *)Ci);
						free((char *)Cj);
						return tmpi->count;
					}
					tmp = get_item(Ci, &Ci_iter);
					strcat(Ci_item, tmp);
					free((char *)tmp);
					strcat(Ci_item, STR_ITEM_SEP);

					if(isfull(Cj_item, 1)) {
						log_msg("Insufficient memory to"
						        " compute %d-candidate",
						        itemset_num);
						free((char *)Ci);
						free((char *)Cj);
						return tmpi->count;
					}
					tmp = get_item(Cj, &Cj_iter);
					strcat(Cj_item, tmp);
					free((char *)tmp);
					strcat(Cj_item, STR_ITEM_SEP);
				}

				/* if they have same n-2 tokens, add together */
				if(strcmp(Ci_item,Cj_item)==0) {

					if(isfull(tmpi->candidate->members, 3))
					{
						log_msg("Insufficient memory to"
						        " generate %d-candidate"
						        , itemset_num);
						free((char *)Ci);
						free((char *)Cj);
						return tmpi->count;
					}
					strcat(tmpi->candidate->members,
					       Ci_item);

					tmp = get_item(Ci, &Ci_iter);
					strcat(tmpi->candidate->members, tmp);
					free((char *)tmp);
					strcat(tmpi->candidate->members,
					       STR_ITEM_SEP);

					tmp = get_item(Cj, &Cj_iter);
					strcat(tmpi->candidate->members, tmp);
					free((char *)tmp);
					strcat(tmpi->candidate->members,
					       STR_ITEMSET_SEP);

					tmpi->count += 1;
				}
				free((char *)Ci);
				free((char *)Cj);
			}
		}
		return tmpi->count;
	}
}

/*----------------------------- PRUNE CANDIDATE ------------------------------*/

/**
 * @brief Forms subsets for given itemset
 * @param members
 * @param itemset_num
 * @return subsets
 * @author SG
 */
static char *get_subsets(char *members,int itemset_num)
{
	char *subset;
	char *token;
	int i,j;

	if(itemset_num == 1) {
		return NULL;
	}

	subset = (char *) malloc(strlen(members) * itemset_num * sizeof(char));
	strcpy(subset, "");

	for(i = (itemset_num - 1) ; i >= 0; i--) {
		for(j = 0; j < itemset_num; j++) {
			if(i == j ) {
				continue;
			}
			token = get_token(members, j, CHAR_ITEM_SEP);
			if(isfull(subset, 1)) {
				log_msg("Insufficient memory to generate "
				       "subsets for %d-candidate", itemset_num);
				subset[strlen(subset) - 1] = CHAR_ITEMSET_SEP;
				return subset;
			}
			strcat(subset,token);
			strcat(subset, STR_ITEM_SEP);
			free((char *) token);
		}
		subset[strlen(subset) - 1] = CHAR_ITEMSET_SEP;
	}
	return subset;
}

/**
 * @brief Removes an item from possible candidates
 * @param tmpi
 * @param itemno
 * @return void
 * @author SG
 */
static void prune_item(I *tmpi, int itemno)
{
	char *tmp;
	char *token;
	int i;

	tmp = (char *) malloc(strlen(tmpi->candidate->members) * sizeof(char));
	strcpy(tmp, "");

	for(i = 0; i < tmpi->count; i++) {
		if(i == itemno) {
			continue;
		}
		token = get_token((char *)tmpi->candidate->members, i,
		                  CHAR_ITEMSET_SEP);
		if(isfull(tmp, 1)) {
			log_msg("Insufficient memory to update pruned "
			        "candidate");
			free((char *) token);
			break;
		} else {
			strcat(tmp, token);
			strcat(tmp, STR_ITEMSET_SEP);
		}
		free((char *) token);
	}
	strcpy(tmpi->candidate->members, tmp);
	free((char*) tmp);
}

/**
 * @brief Discard candidates which do not satisfy Apriori Property
 * @param itemset_num
 * @return Count of candidates remaining after pruning
 * @author SG
 */
static int prune_candidates(int itemset_num)
{
	I *curri, *previ;
	char *subset, *item, *prevtoken, *tmp;
	int i, j, k;

	if(itemset_num == 1) {
		return headi->count;
	}

	previ = get_itemset(itemset_num - 1);
	curri = previ->nexti;

	/* for all candidates in i Itemset */
	for(i = 0; i < curri->count; i++) {

		tmp = get_token(curri->candidate->members, i, CHAR_ITEMSET_SEP);
		subset = get_subsets(tmp, itemset_num);

		 /* log_msg("prune members : %s subset : %s",tmp, subset); */

		/* There are itemset_num subsets in each candidate
		 * check for all subsets */
		for(j = 0; j < itemset_num; j++) {

			item = get_token(subset, j, CHAR_ITEMSET_SEP);

			/* if it is a member of i-1 Itemset */
			for(k = 0; k < previ->count; k++) {

				prevtoken = get_token(previ->candidate->members,
				                     k, CHAR_ITEMSET_SEP);
				if(strcmp(item, prevtoken) == 0) {
					free((char *) prevtoken);
					break; /* element is frequent */
				}
				free((char *) prevtoken);
			}

			if(k == previ->count) { /* Not a frequent element */
				log_msg("Item pruned : %s", tmp);
				prune_item(curri, i);
				i--;
				curri->count -= 1;
				free((char *) item);
				break;
			}

			free((char *) item);
		}
		free((char *) subset);
		free((char *) tmp);
	}

	return curri->count;
}

/*-------------------------- FREQUENT ITEMSETS -------------------------------*/

/**
 * @brief Calculate frequent candidates based on count in Transactions
 * @param itemset_num
 * @param T Total number of transactions
 * @return Count of frequent candidates
 * @author SG
 */
static int calculate_frequent_itemsets(int itemset_num, int T)
{
	I *lasti;
	void *alltag, *tmptag; /* Sqlite3 pointer holding query */
	const char *tname, *fno; /* result string from query */
	char *token, *item;
	int iter;
	int i,j;
	int flag;
	int freqcnt;
	char frequent[MAX_ITEMSET_LENGTH];

	freqcnt = 0;
	strcpy(frequent, "");
	lasti = get_itemset(itemset_num);

	/* check each candidate */
	for(j = 0; j < lasti->count; j++) {
		lasti->candidate->supportcnt[j] = 0;
		token = get_token(lasti->candidate->members, j,
		                  CHAR_ITEMSET_SEP);

		if(itemset_num == 1) {

			/* for each transaction */
			alltag = get_user_tagname();
			while ((tname = string_from_stmt(alltag)) != NULL) {
				/* Analyze the transaction */
				tmptag = get_fid_under_tag(tname);
				while((fno = string_from_stmt(tmptag)) != NULL)
				{
					if(strcmp(fno, token) == 0) {
						lasti->candidate->supportcnt[j]
						     += 1;
						finalize(tmptag);
						break;
					}
				}
			}
			/* log_msg("Frequent 1 token %s : %d", token,
			          lasti->candidate->supportcnt[j]); */
			if(((float)lasti->candidate->supportcnt[j] / T) >
			            MINSUP) {
				if(isfull(frequent, 1)) {
					/** @todo : traverse link list */
					log_msg("Insufficient memory to "
					"generate frequent %d-candidate",
					itemset_num);
					free((char *) token);
					break;
				}
				strcat(frequent, token);
				strcat(frequent, STR_ITEMSET_SEP);
				lasti->candidate->supportcnt[freqcnt] =
				       lasti->candidate->supportcnt[j];
				freqcnt++;
			}
		} else {
			/* for each transaction */
			alltag = get_user_tagname();
			while ((tname = string_from_stmt(alltag)) != NULL) {
				iter = 0;
				for(i = 0; i < itemset_num; i++) {
					flag = 0;
					item = get_item(token, &iter);
					/* Analyze the transaction */
					tmptag = get_fid_under_tag(tname);
					while((fno = string_from_stmt(tmptag))
					      != NULL) {
						if(strcmp(fno, item) == 0) {
							finalize(tmptag);
							flag = 1;
							break;
						}
					}
					if(flag != 1) {
						free((char *) item);
						break;
					}
					free((char *) item);
				}
				if(flag == 1) {
					lasti->candidate->supportcnt[j] += 1;
				}
			}
			/* log_msg("Frequent %d token %s : %d",itemset_num,
			          token, lasti->candidate->supportcnt[j]); */
			if(((float)lasti->candidate->supportcnt[j] / T) >
			            MINSUP) {
				if(isfull(frequent, 1)) {
					/** @todo : traverse link list */
					log_msg("Insufficient memory to "
					"generate frequent %d-candidate",
					itemset_num);
					free((char *) token);
					break;
				}
				strcat(frequent, token);
				strcat(frequent, STR_ITEMSET_SEP);
				lasti->candidate->supportcnt[freqcnt] =
				       lasti->candidate->supportcnt[j];
				freqcnt++;
			}
		}
		free((char *) token);
	}

	/* log_msg("#%d freq : %s #%d cand : %s\n", freqcnt,
	           frequent, lasti->count, lasti->candidate->members); */

	strcpy(lasti->candidate->members, frequent);
	lasti->count = freqcnt;

	return lasti->count;
}

/*-------------------------- ASSOCIATION RULES -------------------------------*/

/**
 * @brief Calculate support count
 * @param item
 * @param itemset_num
 * @return Support count
 * @author SG
 */
static int get_support_count(char *item, int itemset_num)
{
	I *tmpi;
	char *token;
	int i;

	if((tmpi = get_itemset(itemset_num)) == NULL) {
		return -1;
	}

	for(i = 0; i < tmpi->count; i++) {
		token = get_token(tmpi->candidate->members, i,
		                  CHAR_ITEMSET_SEP);
		if(strcmp(token,item) == 0) {
			free((char *) token);
			return tmpi->candidate->supportcnt[i];
		}
		free((char *) token);
	}
	return -1;
}

/**
 * @brief Generate confident subsets
 * @param Nsup Support Count of itemset
 * @param members
 * @param itemset_num
 * @param sub[OUT]
 * @param subcnt[OUT]
 * @return KW_SUCCESS, KW_FAIL, KW_ERROR
 * @author SG
 */
static int get_confident_subsets(int Nsup, char *members, int itemset_num,
                                   char **sub, int *subcnt,int memsize)
{
	char *lhs_itemset = (char *) malloc(strlen(members) * sizeof(char));
	char *token;

	float confidence;
	int Dsup;
	int i, j;

	if(itemset_num == 1) {
		return KW_FAIL;
	}

	/* Calculate subsets */
	for(i = (itemset_num - 1) ; i >= 0; i--) {
		strcpy(lhs_itemset, "");
		for(j = 0; j < itemset_num; j++) {
			if(i == j ) {
				continue;
			}
			token = get_token(members, j, CHAR_ITEM_SEP);
			strcat(lhs_itemset, token);
			free((char *) token);
			strcat(lhs_itemset, STR_ITEM_SEP);
		}
		lhs_itemset[strlen(lhs_itemset) - 1] = '\0';

		/* Get support for subset */
		Dsup = get_support_count(lhs_itemset, (itemset_num - 1));
		if(Dsup == -1) {
			continue;
		}
		/* Calculate confidence in subset */
		confidence = (float) Nsup / Dsup;

		/* Add subset if canfidence is greater then thershold */
		if(confidence > MINCONF) {
			/* check if itemset already present in rule */
			if(check_item(*sub, lhs_itemset, *subcnt,
			               CHAR_ITEMSET_SEP) == 1) {
				continue;
			}
			if(strlen(*sub) > (memsize - strlen(lhs_itemset) - 1)) {
				/* log_msg("Insufficient memory to "
					"generate confident subsets for "
					"%d-candidate",	itemset_num); */
				return KW_ERROR;
			}
			strcat(*sub, lhs_itemset);
			strcat(*sub, STR_ITEMSET_SEP);
			*subcnt += 1;
		}
	}

	free((char*) lhs_itemset);
	return KW_SUCCESS;
}

/**
 * @brief Form complete rules from subsets
 * @param itemset
 * @param itemset_num
 * @param subsets[OUT]
 * @param cnt
 * @return
 * @author SG
 */
static char *complete_rule(char *itemset, int itemset_num, char **subsets,
                            int cnt)
{
	char *rule;
	int item_iter, sub_iter;
	char *itemtmp, *subtmp;
	char *token;

	int i, j, k;
	unsigned int memsize;

	memsize = ((pow(2, itemset_num) * (MAX_ITEM_LENGTH + 3)) + 1);
	rule = (char *) malloc(memsize * sizeof(char));
	strcpy(rule,"");

	/* For each subset */
	for(i = 0; i < cnt; i++) {
		token = get_token(*subsets, i, CHAR_ITEMSET_SEP);
		if(strlen(rule) > (memsize - MAX_ITEM_LENGTH - 2)) {
			/* log_msg("Insufficient memory to complete rule for "
				"%d-candidate",	itemset_num); */
			free((char*) token);
			return rule;
		}

		strcat(rule,token);
		strcat(rule,"->");

		/* for each item in itemset */
		for(j=0, item_iter = 0; j < itemset_num; j++){
			itemtmp = get_item(itemset, &item_iter);
			/* for each item in subset */
			for(k=0, sub_iter=0; k < get_no_of_items(token); k++) {
				subtmp = get_item(token, &sub_iter);
				/* compare each itemset item with every
				 * subset item */
				if(strcmp(itemtmp, subtmp)==0) {
					/* if it is present, do nothing */
					free((char*) subtmp);
					break;
				}
				free((char*) subtmp);
			}
			/* if not present in subset, add item to RHS of rule */
			if(k == get_no_of_items(token)){
				strcat(rule, itemtmp);
				strcat(rule, STR_ITEM_SEP);
			}
			if(itemtmp != NULL) free((char*) itemtmp);
		}
		rule[strlen(rule) - 1] = '\0';
		add_rule(token, strrchr(rule, '>') + 1);
		strcat(rule, STR_ITEMSET_SEP);
		free((char*) token);
	}
	return rule;
}

/**
 * @brief Create Association Rules
 * @param itemset_num
 * @return void
 * @author SG
 */
static void generate_assoc_rule(int itemset_num)
{
	I *tmpi;
	char *maintoken, *token;
	char *subset, *rule;
	int subset_cnt;
	int Nsup;

	int i, j;
	int memsize;
	int status;

	if(itemset_num == 1) {
		return ;
	}

	tmpi = get_itemset(itemset_num);
	memsize = (((pow(2, itemset_num)) - 2) * (MAX_ITEM_LENGTH + 1));
	subset = (char *) malloc(memsize * sizeof(char));

	/* Iterate for all itemsets in Ci */
	for(i = 0; i < tmpi->count; i++) {

		subset_cnt = 0;
		strcpy(subset, "");

		/* For each itemset */
		maintoken = get_token(tmpi->candidate->members, i,
		                      CHAR_ITEMSET_SEP);
		Nsup = get_support_count(maintoken, itemset_num); /* Support */

		/* Calculate Confident Subsets */
		status = get_confident_subsets(Nsup, maintoken, itemset_num,
		                                &subset, &subset_cnt, memsize);

		if(status == KW_SUCCESS) {
			/* Recursively calculate i-1 subsets */
			for(j = 0; j < subset_cnt; j++) {
				token = get_token(subset, j, CHAR_ITEMSET_SEP);

				/*log_msg("%d %d",get_no_of_items(token),
				                  itemset_num);*/
				/* Limit to anslyze subgroups */
				if(get_no_of_items(token) <=
				   (itemset_num * RULE_LEVEL)) {
					break;
				}
				status = get_confident_subsets(Nsup, token,
				         get_no_of_items(token), &subset,
				         &subset_cnt, memsize);
				free((char*) token);
				if(status != KW_SUCCESS) {
					break;
				}
			}
		}

		/* Complete association rule */
		rule = complete_rule(maintoken, itemset_num, &subset,
		                     subset_cnt);

		log_msg("Rule for %s : %s", maintoken ,rule);
		//log_msg("Rule for %s : %s", maintoken ,subset);
		free((char *) rule);
		free((char *) maintoken);
	}

	free((char *) subset);
}

/*------------------------------ APRIORI -------------------------------------*/

/**
 * @brief Display candidates
 * @param itemset_num
 * @return void
 * @author SG
 */
static void display_candidate(int itemset_num)
{
	I *tmpi;
	char *token;
	int i;

	tmpi = get_itemset(itemset_num);

	for(i = 0; i < tmpi->count; i++) {
		token = get_token(tmpi->candidate->members, i,
		                  CHAR_ITEMSET_SEP);
		log_msg("%d] %s", i, token);
		free((char *) token);
	}
}

/**
 * @brief Display candidates
 * @param itemset_num
 * @return void
 * @author SG
 */
static void display_candidate_with_supcnt(int itemset_num)
{
	I *tmpi;
	char *token;
	int i;

	tmpi = get_itemset(itemset_num);

	for(i = 0; i < tmpi->count; i++) {
		token = get_token(tmpi->candidate->members, i,
		                  CHAR_ITEMSET_SEP);
		log_msg("%d] %s\t%d",i, token ,tmpi->candidate->supportcnt[i]);
		free((char *) token);
	}
}

/**
 * @brief Free memory allocated for candidates
 * @param void
 * @return void
 * @author SG
 */
static void free_itemsets(void)
{
	I *tmpi;

	for(tmpi = headi; tmpi != NULL ; tmpi = tmpi->nexti){
		free((C *)tmpi->candidate);
	}

	tmpi = headi;
	while(tmpi != NULL) {
		headi = tmpi->nexti;
		free((I *)tmpi);
		tmpi = headi;
	}
}

/**
 * @brief Apriori Algorithm
 * @param void
 * @return void
 * @author SG
 */
void apriori(void)
{
	int num_transactions; /* number of transactions to be analyzed */

	int itemset_num; /* the current itemset being looked at */
	int candidate_cnt, prune_cnt;

	log_msg("\nInitializing Apriori Algorithm");

	num_transactions = count_user_tags(); /* count of all tags in kwest */
	itemset_num = 1;
	headi = NULL;

	do
	{
		log_msg("\nCandidate %d-itemsets : ", itemset_num);
		candidate_cnt = generate_candidates(itemset_num);
		if (candidate_cnt == 0) {
			log_msg("No candidates generated");
			break;
		}
		display_candidate(itemset_num);

		if(itemset_num > 1) {
			prune_cnt = prune_candidates(itemset_num);
			if (prune_cnt == 0) {
				log_msg("All candidates pruned");
				break;
			}
			if(candidate_cnt != prune_cnt) {
				log_msg("%d-itemsets after Pruning : ",
				         itemset_num);
				display_candidate(itemset_num);
			}
		}

		log_msg("Frequent %d-itemsets : ", itemset_num);
		candidate_cnt = calculate_frequent_itemsets(itemset_num,
		                 num_transactions);
		if (candidate_cnt == 0) {
			log_msg("No frequent candidate identified");
			break;
		}
		display_candidate_with_supcnt(itemset_num);

		if(itemset_num > 1) {
			log_msg("Assocaition Rule %d-itemsets : ", itemset_num);
			generate_assoc_rule(itemset_num);
		}

		itemset_num++;

	}while (candidate_cnt > 1);
	/* If there are <=1 frequent items, then its the end */
	//get_file_suggestions("MyBest");
	free_itemsets();
	log_msg("\nTerminating Apriori Algorithm\n");
}

#ifndef _MYSPELLMGR_H_
#define _MYSPELLMGR_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Hunhandle Hunhandle;

#ifdef _MSC_VER
#define DLL __declspec ( dllexport )
#else
#define DLL 
#endif


DLL Hunhandle *Hunspell_create(const char * affpath, const char * dpath);

DLL Hunhandle *Hunspell_create_key(const char * affpath, const char * dpath,
    const char * key);

DLL void Hunspell_destroy(Hunhandle *pHunspell);

/* spell(word) - spellcheck word
 * output: 0 = bad word, not 0 = good word
 */
DLL int Hunspell_spell(Hunhandle *pHunspell, const char *);

DLL char *Hunspell_get_dic_encoding(Hunhandle *pHunspell);

/* suggest(suggestions, word) - search suggestions
 * input: pointer to an array of strings pointer and the (bad) word
 *   array of strings pointer (here *slst) may not be initialized
 * output: number of suggestions in string array, and suggestions in
 *   a newly allocated array of strings (*slts will be NULL when number
 *   of suggestion equals 0.)
 */
DLL int Hunspell_suggest(Hunhandle *pHunspell, char*** slst, const char * word);

 /* morphological functions */

 /* analyze(result, word) - morphological analysis of the word */

DLL int Hunspell_analyze(Hunhandle *pHunspell, char*** slst, const char * word);

 /* stem(result, word) - stemmer function */

DLL int Hunspell_stem(Hunhandle *pHunspell, char*** slst, const char * word);

 /* stem(result, analysis, n) - get stems from a morph. analysis
  * example:
  * char ** result, result2;
  * int n1 = Hunspell_analyze(result, "words");
  * int n2 = Hunspell_stem2(result2, result, n1);   
  */

DLL int Hunspell_stem2(Hunhandle *pHunspell, char*** slst, char** desc, int n);

 /* generate(result, word, word2) - morphological generation by example(s) */

DLL int Hunspell_generate(Hunhandle *pHunspell, char*** slst, const char * word,
    const char * word2);

 /* generate(result, word, desc, n) - generation by morph. description(s)
  * example:
  * char ** result;
  * char * affix = "is:plural"; // description depends from dictionaries, too
  * int n = Hunspell_generate2(result, "word", &affix, 1);
  * for (int i = 0; i < n; i++) printf("%s\n", result[i]);
  */

DLL int Hunspell_generate2(Hunhandle *pHunspell, char*** slst, const char * word,
    char** desc, int n);

  /* functions for run-time modification of the dictionary */

  /* add word to the run-time dictionary */
  
DLL int Hunspell_add(Hunhandle *pHunspell, const char * word);

  /* add word to the run-time dictionary with affix flags of
   * the example (a dictionary word): Hunspell will recognize
   * affixed forms of the new word, too.
   */
  
DLL int Hunspell_add_with_affix(Hunhandle *pHunspell, const char * word, const char * example);

  /* remove word from the run-time dictionary */

DLL int Hunspell_remove(Hunhandle *pHunspell, const char * word);

  /* free suggestion lists */

DLL void Hunspell_free_list(Hunhandle *pHunspell, char *** slst, int n);

#ifdef __cplusplus
}
#endif

#endif

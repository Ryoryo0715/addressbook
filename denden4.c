/*クオリティCプログラミンング 中級編
**
**例題4:
** 住所録管理表.第4版
**
**file name:denden4.c
**
**コンパイルする場合、ライブラリのリンクオプション -lncursesw をつけること
**
**composed by Naohiko Takeda
**   ver.1.00. 1990.5.30.
**composed by Yoshihide Chubachi
**   ver.1.10. 2003.4.27.
**composed by Shinsuke Kobayashi
**   ver.2.00. 2019.4.11.
**
*/


/*-------------------------------------------*/
/* ヘッダーファイルの読み込み */
/*-------------------------------------------*/

#include <stdio.h>
#include <ctype.h>
#include <ncurses.h>
#include <locale.h>
#include <string.h>
#include <stdlib.h>

/*-----------------------------------------*/
/* システム定数の宣言 */
/*-----------------------------------------*/

#define TRUE  1
#define FALSE  0

#define BUFFER_LEN 80 /*入力バッファの文字数*/

#define RECORD_MAX 20 /* 最大データ件数 */
#define NAME_LEN 40 /* 名前の文字数 */
#define ADDR_LEN 60 /* 住所の文字数 */
#define TEL_LEN 15 /*電話番号の文字数 */
#define ZIP_LEN 9 /*郵便番号の文字数 */

#define TITLE_LINE 1 /* タイトル行の位置 */
#define DATA_LINE 3 /* データ表示行の先頭位置 */
#define DISP_REC_MAX 5 /* 1ページ目に書くデータ件数 */
#define MSG_LINE 18 /* メッセージ行の先頭位置 */

#define UP 1 /* 前ページ */
#define DOWN 0 /* 次ページ */

#define FNAME_MAX 128 /* ファイル名の最大長さ */

/* コマンドの定義 */

#define CMD_MAX 10 /* 第4版 */

#define APPEND 'A' /* データの追加 */
#define EDIT 'E' /* データの編集 */
#define DELETE 'X' /* データの削除 */
#define BEFORE 'B' /* 前のデータへ */
#define NEXT 'N' /* 次のデータへ */
#define UP_PAGE 'U' /* 次ページへ */
#define DOWN_PAGE 'D' /*前ページへ */
#define LOAD 'L' /* データのロード */
#define SAVE 'S' /* データのセーブ */
#define QUIT 'Q' /* 終了 */

char cmd_list[] = {
                    APPEND,
                    EDIT,
                    DELETE,
                    BEFORE,
                    NEXT,
                    UP_PAGE,
                    DOWN_PAGE,
                    LOAD,
                    SAVE,
                    QUIT
                  };


/*-----------------------------------*/
/* グローバル変数の宣言 */
/*-----------------------------------*/

int cursor = 0; /* 注目行 */
int rec_qty = 0; /* データの数 */

typedef struct{
  char j_name[NAME_LEN]; /* 名前（漢字）*/
  char k_name[NAME_LEN]; /* 名前（フリガナ）*/
  char address[ADDR_LEN]; /* 住所 */
  char tel_no[TEL_LEN]; /* 電話番号 */
  char zip[ZIP_LEN]; /* 郵便番号 */
}CARD;

CARD note[RECORD_MAX];

char data_file[FNAME_MAX]; /* データファイル名 */

/*------------------------------------*/
/* 使用する関数の宣言 */
/*------------------------------------*/

/* コマンド処理用部品 */
char input_cmd(void);
void exec_cmd(char cmd);

/* 住所録管理コマンド */
void data_append(void);
void data_edit(void);
void data_delete(void);
void paging(int direction);
void cur_move(int direction);
void data_load(void);
void data_save(void);
void pack_data(int *rec_no, int *tail);

void init_buff();

/* 画面表示用部品 */
void init_disp(void);
void show_one_page(void);
void show_record(int num, int pos);
void del_lines(int start, int end);
void disp_err(char *msg);


/* main */
/*-------------------------------------*/

int main()
{
  char command;

  /* 初期化する */
  init_disp();
  init_buff();
  show_one_page();

  /* コマンドの入力と実行 */
  while((command=input_cmd())!=QUIT){
    exec_cmd(command);
    /* データの再表示 */
    show_one_page();
  }

  /* 後始末する */
  clear();
  endwin();
  return 0;
}


/*-------------------------------*/
/* コマンド処理用部品 */
/*-------------------------------*/

#define CMD_MSG_LINE MSG_LINE
#define CMD_ERR_LINE MSG_LINE+1
#define CMD_LINE MSG_LINE+2

#define CMD_MSG "コマンド(A:追加 E:編集 X:削除 B:前 N:後 U:次貢 D:前貢 L:ロード S:セーブ Q:終了)"
#define CMD_ERR_MSG "不適切なコマンドでした"
#define CMD_PROMPT ">>"


/* input_cmd */
/*--------------------------------------*/
/*
    書式:char input_cmd()

    機能:住所録のコマンドを入力し、妥当なコマンドであれば、入力されたコマ
           ンドを返す。もしも、不適当なコマンドであれば、再入力を要求する。

　　返値:入力されたコマンド
*/

char input_cmd()
{
  int cmd;
  int match=FALSE;
  int i;
  char buffer[BUFFER_LEN];

  /* 前処理 */
  move(CMD_MSG_LINE, 1);
  printw(CMD_MSG);

  /* コマンドの入力 */
  for(; ;){
    move(CMD_LINE, 1); /* コマンドを1文字入力する */
    clrtoeol();
    printw(CMD_PROMPT);
    getstr(buffer);
    cmd = toupper(buffer[0]);
    for(i=0; i<CMD_MAX; i++){ /* 妥当なコマンドか？ */
      if(cmd == cmd_list[i]){
        match=TRUE;
        break;
      }
    }
    if(!match){
      move(CMD_ERR_LINE, 1);
      printw(CMD_ERR_MSG);
    }else{
      break;
    }
  }

  /* 後始末 */
  del_lines(CMD_MSG_LINE, CMD_LINE);
  return(cmd);
}


/* exec_cmd */
/*----------------------------------------*/
/*
    書式: void exec_cmd(char cmd)

    機能: 変数cmdで、指定されたコマンドを実行する。

    返値: なし
*/
void exec_cmd(
              char cmd /* コマンド */
              )
{
  switch(cmd){
  case APPEND:
    data_append(); /* 追加 */
    break;
  case EDIT:
    data_edit(); /* データの編集 */
    break;
  case DELETE:
    data_delete(); /* データの削除 */
    break;
  case UP_PAGE:
    paging(UP); /* 次ページ */
    break;
  case DOWN_PAGE:
    paging(DOWN); /* 前ページ */
    break;
  case NEXT:
    cur_move(UP); /*次のデータ */
    break;
  case BEFORE:
    cur_move(DOWN); /* 前のデータ */
    break;
  case LOAD:
    data_load(); /* データのロード */
    break;
  case SAVE:
    data_save(); /* データのセーブ */
    break;
  default:
    break;
  }
}

/*----------------------------------*/
/* 住所録管理コマンド群 */
/*----------------------------------*/

#define APPEND_LINE MSG_LINE
#define APPEND_END_LINE MSG_LINE+4

#define EDIT_MSG "編集する項目を選んでください N:名前 F:フリガナ A:住所 Z:郵便番号 T:電話 Q:終了"

#define NAME 'N'
#define KANA_NAME 'F'
#define ADDR 'A'
#define ZIP_NO 'Z'
#define TELPHONE 'T'

#define FBUF_MAX 200

#define ERR_APPEND "データがいっぱいで、もう追加できません。"
#define ERR_FILE "ファイルがオープン出来ません。"

#define WATT_MSG "確認したら、好きなキーを押してください。"


/* data_append */
/*------------------------------------------------*/
/*
     書式: void data_append(void)

     機能: 住所録データの末尾に、新しいデータを追加する。

     返値: なし
*/

void data_append(void)
{
  int i;

  move(APPEND_LINE, 1);

  /* データがいっぱいかどうかを調べる。 */
  if(rec_qty >= RECORD_MAX){
    disp_err(ERR_APPEND);
    return;
  }

  /* データを入力する。 */
  printw("名前 >>");
  getnstr(note[rec_qty].j_name, NAME_LEN);
  printw("フリガナ >>");
  getnstr(note[rec_qty].k_name, NAME_LEN);
  printw("住所 >>");
  getnstr(note[rec_qty].address, ADDR_LEN);
  printw("郵便番号 >>");
  getnstr(note[rec_qty].zip, ZIP_LEN);
  printw("電話番号 >>");
  getnstr(note[rec_qty].tel_no, TEL_LEN);

  cursor=++rec_qty;

  /* 後始末 */
  del_lines(APPEND_LINE, APPEND_END_LINE);
}


/* data_edit */
/*-------------------------------------------*/
/*
      書式:void data_edit(void)

      機能:注目行のあるレコードに対して、編集作業を行う。
              まず、レコードのどの項目を修正するか入力する。
              入力された項目について、入力プロンンプトを出力し、全部入力しなおす。
              もとのデータの一部分のみ、修正、変更、削除などは、サポートしない。
              編集項目の選択のところで、終了を選択すると、画面上の注目行のみ、再表示し、
              編集を終了する。

      返値:なし
*/

void data_edit(void)
{
  int disp_pos; /*注目行の表示位置*/
  int c;
  char buffer[BUFFER_LEN];

  if(cursor == 0) /*データがない時の処理 */
    return;

  for(;;){
    move(MSG_LINE, 1); /* 編集項目の選択 */
    printw(EDIT_MSG);
    move(MSG_LINE+2, 1);
    printw(">>");
    getstr(buffer);
    if((c=toupper(buffer[0]))==QUIT)
      break;

    /* 各項目の編集 */
    switch(c){
    case NAME:
      printw("\n名前 >>");
      getnstr(note[cursor-1].j_name, NAME_LEN);
      break;
    case KANA_NAME:
      printw("\nフリガナ >>");
      getnstr(note[cursor-1].k_name, NAME_LEN);
      break;
    case ADDR:
      printw("\n住所 >>");
      getnstr(note[cursor-1].address, ADDR_LEN);
      break;
    case ZIP_NO:
      printw("\n郵便番号 >>");
      getnstr(note[cursor-1].zip, ZIP_LEN);
      break;
    case TELPHONE:
      printw("\n電話番号 >>");
      getnstr(note[cursor-1].tel_no, TEL_LEN);
      break;
    default:
      break;
    }
    /* 注目行の再表示 */
    disp_pos=DATA_LINE+((cursor-1)%DISP_REC_MAX)*3;
    show_record(cursor, disp_pos);
    del_lines(MSG_LINE+2, MSG_LINE+4);
  }
  del_lines(MSG_LINE, MSG_LINE+4);
}


/* data_delete */
/*----------------------------------------------------*/
/*
      書式:void data_delete(void)

      機能:注目行のデータを削除し、空いた所を詰め合わせる。

      返値:なし
*/

void data_delete()
{
  CARD *tmp;

  /* データ無しのチェック */
  if(cursor<=0)
    return;

  /* 注目行の削除 */
  pack_data(&cursor, &rec_qty); /* まず、データの詰め合わせ */
  tmp=&note[rec_qty];
  tmp->k_name[0]='\0'; /* 末尾データの削除 */
  tmp->j_name[0]='\0';
  tmp->zip[0]='\0';
  tmp->address[0]='\0';
  tmp->tel_no[0]='\0';
}

       
/* paging */
/*--------------------------------------------*/
/*
       書式: void paging(int direction)

       機能: 注目行を前、あるいは次ページに移動する。移動できないときは、そのまま。

       返値: なし
*/

void paging(
            int direction /* ページングの方向 */
            )
{
  int next;

  if(rec_qty==0) /* データなし */
    return;

  switch(direction){
  case UP:
    next = cursor+DISP_REC_MAX;
    if(next>rec_qty)
      next=rec_qty;
    break;
  case DOWN:
    next=cursor-DISP_REC_MAX;
    if(next<=0)
      next=1;
    break;
  default:
    break;
  }
  cursor=next;
}


/* cur_move */
/*--------------------------------------*/
/*
     書式: void curove(int direction)

     機能: 注目行を前、あるいは次データに移動する。移動できない時はそのまま。

     返値: なし
*/

void cur_move(
              int direction /* 注目行の移動方向 */
              )
{
  int next;

  if(rec_qty==0) /* データなし */
    return;

  switch(direction){
  case UP:
    next=cursor+1;
    if(next>rec_qty)
      next=rec_qty;
    break;
  case DOWN:
    next=cursor-1;
    if(next<=0)
      next=1;
    break;
  default:
    break;
  }
  cursor=next;
}


/* data_load */
/*--------------------------------------*/
/*
     書式: void data_load(void)

     機能: 住所録データを指定されたファイルからロードする。

     返値: なし
*/

void data_load(void)
{
  char buffer[FBUF_MAX];
  FILE *fp;
  wchar_t name[NAME_LEN], kana[NAME_LEN], zip_tmp[ZIP_LEN], addr_tmp[ADDR_LEN], tel_tmp[TEL_LEN];

  /* ファイル名の入力 */
  move(MSG_LINE, 1);
  printw("ファイルをロードします");
  move(CMD_LINE, 1);
  printw("ファイル名を入力してください >>");
  getstr(data_file);

  /* データファイルからの読み込み */
  if((fp=fopen(data_file, "r"))==NULL){
    disp_err(ERR_FILE);
    return;
  }

  rec_qty=0;
  printw("ロード中");
  while(fgets(buffer, FBUF_MAX, fp)!=NULL){
    addch('*');
    //sscanf(buffer, "%s %s %s %s %s", j_name[rec_qty], k_name[rec_qty], zip[rec_qty], address[rec_qty], tel_no[rec_qty]);
    //
    //
    //
    //
    sscanf(buffer, "%s %s %s %s %s", name, kana, zip_tmp, addr_tmp, tel_tmp);
    wcstombs(note[rec_qty].j_name, name, sizeof(name));
    wcstombs(note[rec_qty].k_name, kana, sizeof(kana));
    wcstombs(note[rec_qty].zip, zip_tmp, sizeof(zip_tmp));
    wcstombs(note[rec_qty].address, addr_tmp, sizeof(addr_tmp));
    wcstombs(note[rec_qty].tel_no, tel_tmp, sizeof(tel_tmp));

    rec_qty++;
  }

  /* 後始末 */
  fclose(fp);
  cursor=1;
  del_lines(MSG_LINE, CMD_LINE+1);
}


/* data_save */
/*--------------------------------------------*/
/*
      書式: void data_save(void)

      機能: 住所録データを指定されたファイルにセーブする

      返値: なし
*/

void data_save(void)
{
  int i, end;
  FILE *fp;

  /* ファイル名の入力 */
  move(MSG_LINE, 1);
  printw("ファイルをセーブします");
  move(CMD_LINE, 1);
  printw("ファイル名を入力してください >>");
  getstr(data_file);

  /* データファイルへの書き出し */
  if((fp=fopen(data_file, "w"))==NULL){
    disp_err(ERR_FILE);
    return;
  }
  end=rec_qty-1;
  printw("セーブ中");
  for(i=0; i<=end; i++){
    fprintf(fp, "%s %s %s %s %s\n", note[i].j_name, note[i].k_name, note[i].zip, note[i].address, note[i].tel_no);
  }

  /* 後始末 */
  fclose(fp);
  del_lines(MSG_LINE, CMD_LINE+1);
}


/* pack_data */
/*-------------------------------------------------*/
/*
     書式: void pack_data(int *rec_no, int *tail)

     機能



     返値
*/

void pack_data(
               int *rec_no, /**/
               int *tail /**/
               )
{
  int i;

  if(*rec_no==*tail){/**/
    *rec_no = *rec_no-1;
    *tail = *tail-1;
    return;
  }
  for(i=*rec_no-1; i<*tail; i++){
    strcpy(note[i].j_name, note[i+1].j_name);
    strcpy(note[i].k_name, note[i+1].k_name);
    strcpy(note[i].zip, note[i+1].zip);
    strcpy(note[i].address, note[i+1].address);
    strcpy(note[i].tel_no, note[i+1].tel_no);
  }
  *tail = *tail-1;
}


/* disp_err */
/*-------------------------------------------------*/
/*
      書式: void disp_err(char *msg)

      機能: 引数で渡されたエラーメッセージを表示し、何かキーが押されるまで待つ。
               キーが押されたら、画面のメッセージ領域をクリアし、戻る。

      返値: なし
*/

void disp_err(
              char *msg /* エラーメッセージ */
              )
{
  char buffer[BUFFER_LEN];

  printw(msg);
  printw(WATT_MSG);
  getstr(buffer);
  del_lines(APPEND_LINE, APPEND_END_LINE);
}

/* init_buff */
/*--------------------------------------*/
/*
       書式: void init_buff()

       機能: 住所録の文字列を初期化する

       返値: なし
*/

void init_buff()
{
  CARD *tmp;
  int i;
  // 住所録のデータの初期化
  for (i=0; i<RECORD_MAX; i++){
    tmp=&note[i];
    tmp->k_name[0] = '\0';
    tmp->j_name[0] = '\0';
    tmp->zip[0] = '\0';
    tmp->address[0] = '\0';
    tmp->tel_no[0] = '\0';
  }
}

/*-------------------*/
/* 画面表示用部品 */
/*-------------------*/

#define LINE "---------------------------------------------------"

#define TITLE_POS 25

#define DATA_START DATA_LINE /* データ領域の開始行 */
#define DATA_END MSG_LINE-1 /* データ領域の最終行 */
#define DATA_STEP 3 /* データの間隔 */


/* init_disp */
/*--------------------------------------------------------------*/
/*
        書式: void init_disp(void)

        機能: 画面を消去し、第1行目にタイトルを書いておく
*/

void init_disp(void)
{
  setlocale(LC_ALL,""); // for UTF-8 setting
  initscr(); // 画面初期化
  clear();
  move(TITLE_LINE, TITLE_POS);
  printw("住所管理表　第1版\n%s", LINE);
}


/* show_one_page */
/*---------------------------------------------*/
/*
       書式: void show_one_page(void)

       機能: 1ページ分の住所録データを1画面に書く。
　　　　　　 1ページには、5件分のデータを書く。

*/

void show_one_page(void)
{
  int start, end, /* 表示開始、終了レコード番号 */
    rec_number, /* 表示するレコード番号 */
    line_pos=DATA_START; /* 表示位置 */
  int i;

  /* 前処理 */
  for(i=DATA_START; i<=DATA_END; i++){
    move(i, 0);
    clrtoeol();
  }
  start=(((cursor-1)/5)*5+1);
  end=start+DISP_REC_MAX-1;
  if(end>rec_qty)
    end=rec_qty;

  /* 5件分のデータを書く。 */
  for(i=start; i<=end; i++){
    show_record(i, line_pos);
    line_pos += DATA_STEP;
  }
}


/* show_record */
/*---------------------------------------------*/

void show_record(
                 int num, /* 表示したいレコード表号 */
                 int pos /* 表示したい行位置 */
                 )
{
  int idx;

  move(pos, 1);

  /* 注目マークをつける */
  if(num==cursor)
    printw("*");
  else
    printw(" ");

  idx=num-1;
  printw("%3d %-20s TEL %-15s \n", num, note[idx].j_name, note[idx].tel_no);
  printw("%-20s 〒%-8s %s\n", note[idx].k_name, note[idx].zip, note[idx].address);
  printw("%s", LINE);
}


/* del_lines */
/*--------------------------------------------------*/
/*
      書式: void del_lines(int start, int end)

      機能: 引数,start,endで指定された行を消す

      返値: なし

*/

void del_lines(start, end)
     int start, end;
{
  int i;

  move(start, 1);
  for(i=start; i<= end; i++){
    clrtoeol();
    addch('\n');
  }
}

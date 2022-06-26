#include "header.h"

#pragma warning(disable:4996)

int ForceShutdown(WCHAR* dest)
{
    /*-----------------------------------------------------------------*/
    // 戻り値
    DWORD dwError = 0;
    // リスタートマネージャーのセッションハンドル
    DWORD pSessionHandle;
    // セッションフラグ(0を設定)
    DWORD dwSessionFlags = 0;
    // セッションキー(要初期化)
    WCHAR strSessionKey[CCH_RM_SESSION_KEY + 1];

    int len = sizeof(strSessionKey) / sizeof(strSessionKey[0]);

    // 初期化
    memset(strSessionKey, 0x00, sizeof(WCHAR) * len);

    dwError = RmStartSession(
        &pSessionHandle,
        dwSessionFlags,
        strSessionKey);

    if (dwError != ERROR_SUCCESS) {
        printf("RmStartSession : %d\n", dwError);
        RmEndSession(pSessionHandle);
        return 1;
    }
    /*-----------------------------------------------------------------*/



    /*-----------------------------------------------------------------*/
    // 登録するファイルの数
    UINT nFiles = 1;
    // 登録するファイル名(絶対パス)
    LPCWSTR rgsFileNames[] = { dest, };
    // 登録するプロセスの数
    UINT nApplications = 0;
    // 構造体RM_UNIQUE_PROCESSの配列
    RM_UNIQUE_PROCESS rgApplications[] = { NULL, };
    // 登録するサービス数
    UINT nServices = 0;
    // 登録するファイル名(相対パス)
    LPCWSTR rgsServiceNames[] = { NULL, };

    dwError = RmRegisterResources(
        dwSessionFlags,
        nFiles,
        rgsFileNames,
        nApplications,
        rgApplications,
        nServices,
        rgsServiceNames
    );

    if (dwError != ERROR_SUCCESS) {
        printf("RmRegisterResources : %d\n", dwError);
        RmEndSession(pSessionHandle);
        return 1;
    }
    /*-----------------------------------------------------------------*/



    /*-----------------------------------------------------------------*/
    // 構造体配列RM_PROCESS_INFOのサイズ
    UINT pnProcInfoNeeded = 0;
    // 構造体配列RM_PROCESS_INFOの数
    UINT pnProcInfo = 0;
    // 構造体配列RM_PROCESS_INF
    RM_PROCESS_INFO* rgAffectedApps = { NULL, };
    // RM_REBOOT_REASONへのポインター
    UINT lpdwRebootReasons;
    // ループ回数上限(試行回数を超えると失敗としてreturn 1)
    int loop_limit = 20;

    while (loop_limit--) {
        dwError = RmGetList(
            pSessionHandle,
            &pnProcInfoNeeded,
            &pnProcInfo,
            rgAffectedApps,
            &lpdwRebootReasons
        );

        // 関数呼び出しが成功したらループを抜ける
        if (dwError == ERROR_SUCCESS) {
            break;
        }

        // メモリを再確保するため一度解放
        free(rgAffectedApps);

        // 構造体(メモリー)のサイズを更新
        pnProcInfo = pnProcInfoNeeded;
        rgAffectedApps = (RM_PROCESS_INFO*)malloc(sizeof(RM_PROCESS_INFO) * pnProcInfo);

        // メモリの確保に失敗したらreturn 1
        if (rgAffectedApps == NULL) {
            printf("unable to allocate memory\n");
            return 1;
        }
    }

    if (dwError != ERROR_SUCCESS) {
        printf("RmGetList : %d\n", dwError);
        RmEndSession(pSessionHandle);
        return 1;
    }
    /*-----------------------------------------------------------------*/


    /*-----------------------------------------------------------------*/
    // shutdownモードを設定
    ULONG lActionFlags = RmForceShutdown;

    dwError = RmShutdown(
        pSessionHandle,
        lActionFlags,
        NULL);

    if (dwError != ERROR_SUCCESS) {
        printf("RmShutdown : %d\n", dwError);
        RmEndSession(pSessionHandle);
        return 1;
    }

    RmEndSession(pSessionHandle);
    /*-----------------------------------------------------------------*/

    return 0;
}

// プログラムの実行: Ctrl + F5 または [デバッグ] > [デバッグなしで開始] メニュー
// プログラムのデバッグ: F5 または [デバッグ] > [デバッグの開始] メニュー

// 作業を開始するためのヒント: 
//    1. ソリューション エクスプローラー ウィンドウを使用してファイルを追加/管理します 
//   2. チーム エクスプローラー ウィンドウを使用してソース管理に接続します
//   3. 出力ウィンドウを使用して、ビルド出力とその他のメッセージを表示します
//   4. エラー一覧ウィンドウを使用してエラーを表示します
//   5. [プロジェクト] > [新しい項目の追加] と移動して新しいコード ファイルを作成するか、[プロジェクト] > [既存の項目の追加] と移動して既存のコード ファイルをプロジェクトに追加します
//   6. 後ほどこのプロジェクトを再び開く場合、[ファイル] > [開く] > [プロジェクト] と移動して .sln ファイルを選択します

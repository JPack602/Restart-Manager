#include "header.h"

#pragma warning(disable:4996)

int ForceShutdown(WCHAR* dest)
{
    /*-----------------------------------------------------------------*/
    // �߂�l
    DWORD dwError = 0;
    // ���X�^�[�g�}�l�[�W���[�̃Z�b�V�����n���h��
    DWORD pSessionHandle;
    // �Z�b�V�����t���O(0��ݒ�)
    DWORD dwSessionFlags = 0;
    // �Z�b�V�����L�[(�v������)
    WCHAR strSessionKey[CCH_RM_SESSION_KEY + 1];

    int len = sizeof(strSessionKey) / sizeof(strSessionKey[0]);

    // ������
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
    // �o�^����t�@�C���̐�
    UINT nFiles = 1;
    // �o�^����t�@�C����(��΃p�X)
    LPCWSTR rgsFileNames[] = { dest, };
    // �o�^����v���Z�X�̐�
    UINT nApplications = 0;
    // �\����RM_UNIQUE_PROCESS�̔z��
    RM_UNIQUE_PROCESS rgApplications[] = { NULL, };
    // �o�^����T�[�r�X��
    UINT nServices = 0;
    // �o�^����t�@�C����(���΃p�X)
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
    // �\���̔z��RM_PROCESS_INFO�̃T�C�Y
    UINT pnProcInfoNeeded = 0;
    // �\���̔z��RM_PROCESS_INFO�̐�
    UINT pnProcInfo = 0;
    // �\���̔z��RM_PROCESS_INF
    RM_PROCESS_INFO* rgAffectedApps = { NULL, };
    // RM_REBOOT_REASON�ւ̃|�C���^�[
    UINT lpdwRebootReasons;
    // ���[�v�񐔏��(���s�񐔂𒴂���Ǝ��s�Ƃ���return 1)
    int loop_limit = 20;

    while (loop_limit--) {
        dwError = RmGetList(
            pSessionHandle,
            &pnProcInfoNeeded,
            &pnProcInfo,
            rgAffectedApps,
            &lpdwRebootReasons
        );

        // �֐��Ăяo�������������烋�[�v�𔲂���
        if (dwError == ERROR_SUCCESS) {
            break;
        }

        // ���������Ċm�ۂ��邽�߈�x���
        free(rgAffectedApps);

        // �\����(�������[)�̃T�C�Y���X�V
        pnProcInfo = pnProcInfoNeeded;
        rgAffectedApps = (RM_PROCESS_INFO*)malloc(sizeof(RM_PROCESS_INFO) * pnProcInfo);

        // �������̊m�ۂɎ��s������return 1
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
    // shutdown���[�h��ݒ�
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

// �v���O�����̎��s: Ctrl + F5 �܂��� [�f�o�b�O] > [�f�o�b�O�Ȃ��ŊJ�n] ���j���[
// �v���O�����̃f�o�b�O: F5 �܂��� [�f�o�b�O] > [�f�o�b�O�̊J�n] ���j���[

// ��Ƃ��J�n���邽�߂̃q���g: 
//    1. �\�����[�V���� �G�N�X�v���[���[ �E�B���h�E���g�p���ăt�@�C����ǉ�/�Ǘ����܂� 
//   2. �`�[�� �G�N�X�v���[���[ �E�B���h�E���g�p���ă\�[�X�Ǘ��ɐڑ����܂�
//   3. �o�̓E�B���h�E���g�p���āA�r���h�o�͂Ƃ��̑��̃��b�Z�[�W��\�����܂�
//   4. �G���[�ꗗ�E�B���h�E���g�p���ăG���[��\�����܂�
//   5. [�v���W�F�N�g] > [�V�������ڂ̒ǉ�] �ƈړ����ĐV�����R�[�h �t�@�C�����쐬���邩�A[�v���W�F�N�g] > [�����̍��ڂ̒ǉ�] �ƈړ����Ċ����̃R�[�h �t�@�C�����v���W�F�N�g�ɒǉ����܂�
//   6. ��قǂ��̃v���W�F�N�g���ĂъJ���ꍇ�A[�t�@�C��] > [�J��] > [�v���W�F�N�g] �ƈړ����� .sln �t�@�C����I�����܂�

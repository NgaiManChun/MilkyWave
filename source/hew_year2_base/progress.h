// =======================================================
// progress.h
// 
// 0.0f����ړI�l�܂ŗ݌v���āA
// ���ݒl�ƖړI�l�̔䗦0.0f�`1.0f�Ƃ��Ďg����N���X
// 
// ��ҁF鰕��r�i�K�C�@�}���`�����j�@2024/07/06
// 
// 2024/12/27 �uoperator =�v�ǉ�
// =======================================================
#ifndef _PROGRESS_H
#define _PROGRESS_H

class Progress {
private:
	float _value;		// ���ݒl
	float _duration;	// �ړI�l�A�}�C�i�X�s��

public:
	// �p�u���b�N�ϐ�
	bool loop;			// ���ݒl���ړI�l�����������[�v�����邩

	// �p�u���b�N�֐�
	Progress() :
		Progress(0.0f, false)
	{}
	Progress(float duration, bool loop) :
		_value(0.0f),
		_duration(duration),
		loop(loop)
	{}
	void SetValue(float value);
	float GetValue();
	void SetDuration(float duration);
	float GetDuration();
	operator float() const;
	void operator =(const float t);

	// ���ݒl�ɉ��Z����֐�
	float IncreaseValue(float value);
	
};

#endif
// =======================================================
// progress.cpp
// 
// 0.0f����ړI�l�܂ŗ݌v���āA
// ���ݒl�ƖړI�l�̔䗦0.0f�`1.0f�Ƃ��Ďg����N���X
// 
// ��ҁF鰕��r�i�K�C�@�}���`�����j�@2024/07/06
// =======================================================
#include "progress.h"

void Progress::SetValue(float value) 
{
	_value = value;
	
	// 0.0f�`�ړI�l�܂ŁA�͂ݏo�����Ȃ�
	if (loop) {
		while (_value > _duration) {
			_value -= _duration;
		}
		while (_value < 0) {
			_value += _duration;
		}
	}
	else if (_value > _duration) {
		_value = _duration;
	}
	else if (_value < 0.0f) {
		_value = 0.0f;
	}
}

float Progress::GetValue() 
{
	return _value;
}

void Progress::SetDuration(float duration) 
{
	if (_duration > 0.0f) {
		_duration = duration;
		SetValue(_value);	// �͂ݏo������
	}
}

float Progress::GetDuration() {
	return _duration;
}

// =======================================================
// ���ݒl�ɉ��Z����֐�
// =======================================================
float Progress::IncreaseValue(float value) 
{
	SetValue(GetValue() + value);
	return _value / _duration;
}

// =======================================================
// float�Ƃ��Ďg�����̋���
// =======================================================
Progress::operator float() const
{
	return _value / _duration;
}

void Progress::operator=(const float t)
{
	 SetValue(_duration * t);
}

// =======================================================
// drawToolDX.h
// 
// DirectX���̃X�v���C�g�c�[��
// 
// ��ҁF鰕��r�i�K�C�@�}���`�����j�@2024/11/06
// =======================================================
#ifndef _DRAW_TOOL_DX_H
#define _DRAW_TOOL_DX_H

#include "drawTool.h"
#include "rendererDX.h"
#include "resourceToolDX.h"
#include <map>
#include <string>

namespace MG {

	class DrawToolDX : public DrawTool {
	protected:
		RendererDX* renderer;
		void DrawModelNode(ModelDX* model, MODEL_NODE* const node, const std::map<MODEL_NODE*, M4x4>& nodeWorldTransforms);
	public:
		DrawToolDX(RendererDX* renderer);
		~DrawToolDX();
		void DrawQuad(const Texture* texture, const F3& position, const F3& size, const Quaternion& rotate, const F4& color, const F2& uvOffset, const F2& uvRange) override;
		void DrawCube(const Texture* texture, const F3& position, const F3& size, const Quaternion& rotate, const F4& color) override;
		void DrawModel(const Model* model, const F3& position, const F3& size, const Quaternion& rotate, const F4& color, const F2& uvOffset, const F2& uvRange) override;
		void DrawModel(const Model* model, const std::vector<ANIMATION_APPLICANT>& animationApplicants, const F3& position, const F3& size, const Quaternion& rotate, const F4& color) override;
		void DrawPolygon(const Texture* texture, const VERTEX* vertices, size_t length, TOPOLOGY topology, const F3& position, const F3& size, const Quaternion& rotate, const F4& color, const F2& uvOffset, const F2& uvRange) override;
		void DrawCube(const M4x4& matrix, const F4& color = { 1.0f, 1.0f, 1.0f, 1.0f }) override;
	};

} // namespace MG

#endif
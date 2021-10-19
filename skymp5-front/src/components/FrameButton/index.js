import React from 'react';
// import { ReactComponent as BackgroundFrameRight } from '';
// import BackgroundFrameRight from '';

import './styles.scss'


const ButtonItem = props => {
    const height = props.height != undefined ? props.height : 64;
    const width = props.width != undefined ? props.width : 64;
    const url = props.url || require(`./img/${props.name}.svg`).default;
    const isDefault = props.variant === undefined;
    const isMiddleLeft = props.variant === 'middle_left';
    const isMiddleRight = props.variant === 'middle_right'

    return (
        <div
            style={{
                backgroundImage: `url(${url})`,
                backgroundSize: `${width}px ${height}px`,
                backgroundRepeat: 'repeat',
                height:`${height}px`,
                width: `${width}px`,
            }}
            className={props.name.replace(' ', '-')}
        >
            {isDefault &&
                (props.text)
                    ?
                    <span className={'button-middle--text'} style={{ maxHeight: `${height}px`, width: `${width}px` }}>{props.text}</span>
                    :
                    ''
            }
            {
                !isDefault && (props.text) ? <span
                    className={`${isMiddleLeft ? 'button-middle--left' : ''}${isMiddleRight ? 'button-middle--right' : ''}`}
                    style={{ maxHeight: `${height}px`, width: `${width}px` }}
                >
                    {props.text}
                </span> : ''
            }
        </div>
    )
}

const FrameButton = props => {
    let fwidth = props.width || 384;
    let fheight = props.height || 64;
    let idDisabled = props.disabled != undefined ? props.disabled : true;
    const isDefault = props.variant === 'DEFAULT';
    const isFrameLeft = props.variant === 'LEFT';
    const isFrameRight = props.variant === 'RIGHT';
    const border = () => {
        if (isFrameLeft) return '2px solid red';
        if (isFrameRight) return '2px solid blue';
    }

    return (
        <>
            {isDefault && <div
                className={`skymp-button ${idDisabled ? 'disabled' : 'active'}`}
                onClick={(e) => {
                    if (!props.disabled)
                        props.onClick ? props.onClick(e) : console.log(e)
                }}
                style={{ height: `${fheight}px`, width: `${fwidth}px` }}>
                <ButtonItem name={`button start${idDisabled ? ' disabled' : ''}`} height={fheight} />
                <ButtonItem name={`button middle${idDisabled ? ' disabled' : ''}`} width={fwidth - 64 * 2} height={fheight} text={props.text} />
                <ButtonItem name={`button end${idDisabled ? ' disabled' : ''}`} height={fheight} />
            </div>}
            {isFrameLeft && <div
                className={`skymp-button ${idDisabled ? 'disabled' : 'active'}`}
                onClick={(e) => {
                    if (!props.disabled)
                        props.onClick ? props.onClick(e) : console.log(e)
                }}
                style={{ height: `${fheight}px`, width: `${fwidth}px` }}>
                <ButtonItem name={`button start${idDisabled ? ' disabled' : ''}`} height={fheight} />
                <ButtonItem variant='middle_left' name={`button middle${idDisabled ? ' disabled' : ''}`} width={fwidth - 64 * 2} height={fheight} text={props.text} />
                <ButtonItem name={`frame_button_end${idDisabled ? ' disabled' : ''}`} height={fheight} />
            </div>}
            {isFrameRight && <div
                className={`skymp-button ${idDisabled ? 'disabled' : 'active'}`}
                onClick={(e) => {
                    if (!props.disabled)
                        props.onClick ? props.onClick(e) : console.log(e)
                }}
                style={{ height: `${fheight}px`, width: `${fwidth}px` }}>
                <ButtonItem name={`frame_button_start${idDisabled ? ' disabled' : ''}`} height={fheight} />
                <ButtonItem variant='middle_right' name={`button middle${idDisabled ? ' disabled' : ''}`} width={fwidth - 64 * 2} height={fheight} text={props.text} />
                <ButtonItem name={`button end${idDisabled ? ' disabled' : ''}`} height={fheight} />
            </div>}
        </>
    )
}

export default FrameButton;

import React, {useEffect, useState} from 'react';
import SkyrimButton from "../../components/SkyrimButton";
import SkyrimInput from "../../components/SkyrimInput";
import {toggleClass} from "../../utils/toggleClass";


const LoginForm = props => {
    const [data, setData] = useState({
        email: '',
        password: ''
    })
    const [isButtonDisabled, setButtonDisabled] = useState(true)
    const [isRemember, setRemember] = useState(true)
    const handleInput = (e) => {
        setData({...data, [e.target.name]: e.target.value})
        if (data.email.length > 5 && data.password.length > 3) {
            console.log(true)
            setButtonDisabled(false)
        }
        else {
            setButtonDisabled(true)
        }
        console.log(data)
    }
    const handleSubmit = () => {
        console.log('submit', data)
    }
    useEffect(() => {
        const listener = (e) => {
            console.log(e.key, isButtonDisabled, e.key === 'Enter')
            if (e.key === 'Enter' && !isButtonDisabled) {
                handleSubmit()
            }
        }
        document.addEventListener('keypress', listener)
        return () => document.removeEventListener('keypress', listener)
    }, [data, isButtonDisabled])
    return (
        <div className={'login-form--content_main'}>
            <div className={'login-form--content_main__email'}>
                <div className={'login-form--content_main__label'}>
                    <span>{props.locale.LOGIN.EMAIL}</span>
                    <img src={require('../../img/mail.svg').default} alt=""/>
                </div>
                <SkyrimInput onInput={handleInput} placeholder={props.locale.LOGIN.EMAIL_PLACEHOLDER} type={'text'} name={'email'}/>

            </div>
            <div className={'login-form--content_main__password'}>
                <div className={'login-form--content_main__label'}>
                    <span>{props.locale.LOGIN.PASSWORD}</span>
                    <img src={require('../../img/password.svg').default} alt=""/>
                </div>
                <SkyrimInput onInput={handleInput} placeholder={props.locale.LOGIN.PASSWORD_PLACEHOLDER} type={'password'} name={'password'}/>
            </div>
            <div className={'login-form--content_main__footer'}>
                <div className={'login-form--content_main__label login-form--content_main__container'}>
                    <span>{props.locale.LOGIN.REMEMBER_PLACEHOLDER}</span>
                    <label htmlFor="cbtest" className={"checkbox active"} onClick={(e) => {
                        if (isRemember) setRemember(false)
                        else setRemember(true)
                        toggleClass(e.target, 'active')
                    }}/>
                </div>
                <div
                    className={`skymp-input button ${!isButtonDisabled ? 'disabled' : ''}`}
                    onClick={() => console.log('switch to register')}
                >
                    <span className={'skymp-input_text'}>{props.locale.LOGIN.REGISTER_BUTTON}</span>
                </div>
            </div>
            <div className={'login-form--content_main__button'}>
                <SkyrimButton disabled={isButtonDisabled} onClick={(e) => {console.log(data)}} text={props.locale.LOGIN.LOGIN_BUTTON_TEXT}/>
            </div>
        </div>
    )


}

export default LoginForm